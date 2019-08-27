//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include "helpers.h"

using namespace uwp;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Provider;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// Audio output should be provided here.
static bool audioProcessing(void *clientData, float *audio, int numberOfSamples, int sampleRate) {
	auto data = reinterpret_cast<ClientData*>(clientData);
	return data->OnProcess(clientData, audio, numberOfSamples, sampleRate);
}

// Handle player events here.
static void playerCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
	auto data = reinterpret_cast<ClientData*>(clientData);
	switch (event) {
	case SuperpoweredAdvancedAudioPlayerEvent_EOF:
		break;
	case SuperpoweredAdvancedAudioPlayerEvent_ProgressiveDownloadError:
	case SuperpoweredAdvancedAudioPlayerEvent_HLSNetworkError:
	case SuperpoweredAdvancedAudioPlayerEvent_LoadError:
	{
		char msg[MAX_PATH + 1];
		snprintf(msg, MAX_PATH, "Load error: %s", static_cast<const char *>(value));
		data->OnPlayerLoadError(msg);
		break;
	}
	case SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess:
		data->OnPlayerLoaded();
		break;
	default:;
	}
}

static void recorderStopped(void *clientdata) {
	Windows::System::Launcher::LaunchFolderAsync(Windows::Storage::ApplicationData::Current->TemporaryFolder);
}

void ClientData::OnPlayerLoaded() {
	mMainPage->Dispatcher->RunAsync(
		Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler(
			[this]() {
				mMainPage->txtLoadStatus->Text = "Load success.";
				mMainPage->btnAction->IsEnabled = true;
				mMainPage->sldSeek->Maximum = mMainPage->mSuperpowered.player->durationSeconds;
				mMainPage->showPlayerDuration();
			}
		)
	);
}

void ClientData::OnPlayerLoadError(const char *errorMessage) {
	wchar_t msg[MAX_PATH + 1];
	charToWchar(errorMessage, msg);
	mMainPage->txtLoadStatus->Text = ref new Platform::String(msg);
}

bool ClientData::OnProcess(void *clientData, float *audio, int numberOfSamples, int sampleRate) {
	if (!audio) {
		if (sampleRate == 0) {
			Log("Audio I/O stopped.\n");
			mMainPage->Dispatcher->RunAsync(
				Windows::UI::Core::CoreDispatcherPriority::Normal,
				ref new Windows::UI::Core::DispatchedHandler(
					[this]() {
						mMainPage->stop();
					}
				)
			);
		}
		else {
			Log("Audio I/O error %i.\n", sampleRate);
		}
		return false;
	}

	mMainPage->mSuperpowered.setSampleRate(sampleRate);

	const bool processResult = mMainPage->mSuperpowered.process(audio, numberOfSamples);

	//	Quickhack to force update once per second and avoid collision with slider
	if (mMainPage->mSuperpowered.mLastPosition != mMainPage->mSuperpowered.player->positionSeconds) {
		mMainPage->Dispatcher->RunAsync(
			Windows::UI::Core::CoreDispatcherPriority::Normal,
			ref new Windows::UI::Core::DispatchedHandler(
				[this]() {
					mMainPage->sldSeek->Value = mMainPage->mSuperpowered.player->positionSeconds;
					mMainPage->showPlayerDuration();
				}
			)
		);
	}
	mMainPage->mSuperpowered.mLastPosition = mMainPage->mSuperpowered.player->positionSeconds;

	return processResult;
}

bool SuperpoweredWrapper::process(float *audio, int numberOfSamples) {
	player->process(audio, false, numberOfSamples, mVolume);
	reverb->process(audio, audio, numberOfSamples);
	echo->process(audio, audio, numberOfSamples);
	recorder->process(audio, numberOfSamples);

	return true;
}

void SuperpoweredWrapper::setSampleRate(int sampleRate) {
	if (mLastSamplerate != sampleRate) {
		mLastSamplerate = sampleRate;

		player->setSamplerate(sampleRate);
		reverb->setSamplerate(sampleRate);
		echo->setSamplerate(sampleRate);
		recorder->setSamplerate(sampleRate);
	}
}

MainPage::MainPage() 
	: mAudioIO(audioProcessing, &mClientData, false, true)
{
	formatTimeString(218);
	formatTimeString(3600);
	formatTimeString(4000);
	formatTimeString(10000);
	mSuperpowered.reverb = std::make_unique<SuperpoweredReverb>(mSuperpowered.mLastSamplerate);
	mSuperpowered.echo = std::make_unique<SuperpoweredEcho>(mSuperpowered.mLastSamplerate);

	InitializeComponent();

	auto tempPath = String::Concat(Windows::Storage::ApplicationData::Current->TemporaryFolder->Path, "\\temp.wav");
	DeleteFile(tempPath->Data());
	char path[MAX_PATH + 1];
	stringToChar(tempPath, path);
	mSuperpowered.recorder = std::make_unique<SuperpoweredRecorder>(path, mSuperpowered.mLastSamplerate, 1, 2, false, recorderStopped, nullptr);

	stringToChar(String::Concat(Windows::Storage::ApplicationData::Current->TemporaryFolder->Path, "\\New Wave"), mSavePath);

	mSuperpowered.player = std::make_unique<SuperpoweredAdvancedAudioPlayer>(&mClientData, playerCallback, mSuperpowered.mLastSamplerate, 0);
}

void MainPage::chkEcho_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	mSuperpowered.echo->enable(chkEcho->IsChecked->Value);
}

void MainPage::sldEchoMix_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.echo->setMix(static_cast<float>(e->NewValue));
}

void MainPage::sldEchoBeats_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.echo->beats = static_cast<float>(e->NewValue);
}

void MainPage::sldEchoBpm_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.echo->bpm = static_cast<float>(e->NewValue);
}

void MainPage::sldEchoDecay_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.echo->decay = static_cast<float>(e->NewValue);
}

void MainPage::chkReverb_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	mSuperpowered.reverb->enable(chkReverb->IsChecked->Value);
}

void MainPage::sldReverbDry_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.reverb->setDry(static_cast<float>(e->NewValue));
}

void MainPage::sldReverbWet_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.reverb->setWet(static_cast<float>(e->NewValue));
}

void MainPage::sldReverbWidth_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.reverb->setWidth(static_cast<float>(e->NewValue));
}

void MainPage::sldReverbDamp_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.reverb->setDamp(static_cast<float>(e->NewValue));
}

void MainPage::sldReverbRoom_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.reverb->setRoomSize(static_cast<float>(e->NewValue));
}

void MainPage::sldReverbPredelay_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.reverb->setPredelay(static_cast<float>(e->NewValue));
}

void MainPage::btnOpen_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
    FileOpenPicker^ openPicker = ref new FileOpenPicker();
    openPicker->ViewMode = PickerViewMode::List;
    //openPicker->SuggestedStartLocation = PickerLocationId::MusicLibrary;
    openPicker->FileTypeFilter->Append(".wav");
    openPicker->FileTypeFilter->Append(".mp3");

    create_task(openPicker->PickSingleFileAsync()).then(
		[this](StorageFile^ file) {
			if (file) {
				char path[MAX_PATH + 1];
				stringToChar(file->Name, path);
				txtOpenFile->Text = file->Path;
				mSuperpowered.player->open(path);
			}
		}
	);
}

void MainPage::btnAction_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	if (!mSuperpowered.mIsPlaying) {
		mAudioIO.start();
		mSuperpowered.player->play(false);

		if (!DeleteFileA(mSavePath)) {
			Log("Failed to delete old file: %d\n", GetLastError());
		}
		mSuperpowered.recorder->start(mSavePath);

		btnAction->Content = "Stop and Save";
		mSuperpowered.mIsPlaying = true;
		mSuperpowered.mLastPosition = 0;
	} else {
		stop();
	}
}

void MainPage::sldVolume_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.mVolume = static_cast<float>(e->NewValue);
}

void MainPage::sldSeek_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e) {
	mSuperpowered.player->setPosition(static_cast<float>(e->NewValue) * 1000, false, false);
	showPlayerDuration();
}

void MainPage::stop() {
	mSuperpowered.recorder->stop();
	mSuperpowered.player->seek(0);
	mAudioIO.stop();

	btnAction->Content = "Play and Record";
	mSuperpowered.mIsPlaying = false;
}

void MainPage::showPlayerDuration() {
	wchar_t msg[MAX_PATH + 1];
	_snwprintf_s(
		msg,
		MAX_PATH,
		L"%s / %s",
		formatTimeString(mSuperpowered.player->positionSeconds).c_str(),
		formatTimeString(mSuperpowered.player->durationSeconds).c_str()
	);
	txtTrackProgress->Text = ref new Platform::String(msg);
}