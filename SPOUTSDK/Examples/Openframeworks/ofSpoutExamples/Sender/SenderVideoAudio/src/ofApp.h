/*

	Spout OpenFrameworks Video/Audio Sender example

	Copyright (C) 2026 Lynn Jarvis.

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#pragma once

#include "ofMain.h"
#include "ofxWinMenu.h" // Windows menu
#include "../../../apps/SpoutGL/Spout.h" // Spout SDK

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);
		void dragEvent(ofDragInfo dragInfo);
		void audioOut(ofSoundBuffer &buffer);

		ofTexture myTexture; // Draw texture
		ofSoundStream soundStream; // To get sound to speakers
		ofTrueTypeFont myFont;

		// Menu
		HWND m_hWnd = nullptr;
		ofxWinMenu* menu = nullptr; // Menu object
		void appMenuFunction(std::string title, bool bChecked); // Menu callback function
		bool bMute = false;
		bool bScale = true;
		bool bPaused = false;
		bool bRestart = false;
		bool bStop = false;
		bool bShowInfo = true;

		// Sender
		Spout sender;  // Sender object
		char m_SenderName[256]{}; // Sender name
		unsigned int m_SenderWidth = 1280; // Sender width (video width)
		unsigned int m_SenderHeight = 720; // Sender height (video height)

		// FFmpeg
		std::string m_exePath;           // Executable location
		std::string m_ffmpegPath;        // FFmpeg location

		// Video
		std::string m_videopath;         // The full video path
		double m_FrameRate = 30.0;       // Video frame rate
		double m_SenderFps = 0.0;        // For fps display averaging
		FILE *m_pipein = nullptr;        // Pipe for FFmpeg video
		std::string m_codecName;         // Codec name
		std::string m_input;             // Input string to FFmpeg video
		unsigned char *m_pixelBuffer = nullptr; // RGBA pixel buffer

		// Audio
		std::string m_audioInput;         // Input string to FFmpeg audio
		FILE* m_audioPipe = nullptr;      // Audio pipe
		float* m_audiodata = nullptr;     // Audio data
		std::vector<int> audiosamples;    // Sequence of sample numbers per frame
		std::vector<char> m_audioBuffer;  // The audio buffer used in audioOut TODO
		std::vector<int16_t> m_pcmBuffer; // PCM data buffer used in audioOut

		// LJ DEBUG
		std::atomic<double> audioClock = 0.0;

		int m_nChannels = 0;              // Number of channels (2 for stereo) 
		int m_sampleRate = 0;             // Audio sample rate

		// For audio pause with menu selection or title bar click
		bool bNCmousePressed = false;
		bool bAudioRead = false; // Flag for Draw

		bool OpenVideo(std::string filePath); // Open a video with FFmpeg
		void CloseVideo();                    // Release FFmpeg resources
		bool OpenSender();                    // Open sender
		bool ffprobe(std::string filePath);   // Get video file information
		void ResetWindow();
		void ShowInfo();
};
