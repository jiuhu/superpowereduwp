//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

#include <SuperpoweredWindowsAudioIO.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include <SuperpoweredReverb.h>
#include <SuperpoweredEcho.h>
#include <SuperpoweredRecorder.h>

#include <memory>

namespace uwp {
	constexpr int kDefaultSampleRate = 44100;

	class ClientData {
	public:
		ClientData(MainPage^ mainPage)
			: mMainPage(mainPage)
		{}

		void OnPlayerLoaded();
		void OnPlayerLoadError(const char *errorMessage);
		bool OnProcess(void *clientData, float *audio, int numberOfSamples, int sampleRate);

	private:
		MainPage^ mMainPage;
	};

	class SuperpoweredWrapper {
	public:
		bool process(float *audio, int numberOfSamples);
		void setSampleRate(int sampleRate);

		std::unique_ptr<SuperpoweredAdvancedAudioPlayer> player;
		std::unique_ptr<SuperpoweredReverb> reverb;
		std::unique_ptr<SuperpoweredEcho> echo;
		std::unique_ptr<SuperpoweredRecorder> recorder;
		int mLastSamplerate{ kDefaultSampleRate };
		unsigned int mLastPosition{ 0 };
		float mVolume{ 1.0f };
		bool mIsPlaying{ false };
	};

	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void chkEcho_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void sldEchoMix_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldEchoBeats_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldEchoBpm_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldEchoDecay_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		void chkReverb_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void sldReverbDry_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldReverbWet_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldReverbWidth_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldReverbDamp_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldReverbRoom_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldReverbPredelay_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		void btnOpen_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnAction_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void sldVolume_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sldSeek_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		void stop();
		void showPlayerDuration();

		char mSavePath[MAX_PATH + 1];
		bool mIsSeeking{ false };

		friend class ClientData;
		ClientData mClientData{this};
		
		SuperpoweredWindowsAudioIO mAudioIO;
		SuperpoweredWrapper mSuperpowered;
	};
}
