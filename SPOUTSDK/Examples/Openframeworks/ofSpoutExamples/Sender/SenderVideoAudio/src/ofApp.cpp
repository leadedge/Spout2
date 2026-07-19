/*

	Spout OpenFrameworks Video/Audio Sender example

	An example of a sender for video and audio using FFmpeg
	to open two pipes, one for video and the second for audio.
	This is a simple method, although seeking is not supported.

	Uses the ofxWinMenu addon to create a menu and manage
	caption mouse press. Source is included for convenience.

	The example may be useful for further reference :

	o Using ofxWinMenu to create a window menu
	o Detecting non-client area mouse press
    o FFprobe to read video file details
    o FFmpeg with a pipe to decode video and audio frames
    o Openframeworks dragEvent for drag and drop
	o Openframeworks soundstream and audioOut
	o Fps control using "HoldFps" with FFmpeg constant frame rate
	o Draw and position ofTrueTypeFont text
    o SetSenderName, SendImage, LoadTexturePixels and ReleaseSender
	o Utility OpenSpoutConsole and SpoutMessageBox functions
    
	FFmpeg and FFprobe are required.
	Refer to bin/data/ffmpeg/readme.md

	Copyright (C) 2026 Lynn Jarvis.

	=========================================================================
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
	=========================================================================
*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	// OpenSpoutConsole(); // for debugging

	// Set the sender name
	strcpy_s(m_SenderName, 256, "Video Audio Sender");
	sender.SetSenderName(m_SenderName);

	 // show it on the title bar
	ofSetWindowTitle(m_SenderName);

	// Load a font rather than the default
	myFont.load("fonts/verdana.ttf", 12, true, true);

	// Executable location
	char exePath[MAX_PATH]{};
	GetModuleFileNameA(NULL, exePath, MAX_PATH); // Path of the executable
	PathRemoveFileSpecA(exePath);
	m_exePath = exePath;

	// FFmpeg location - /data/ffmpeg/
	m_ffmpegPath = m_exePath;
	m_ffmpegPath += "/data/ffmpeg/ffmpeg.exe";
	if (_access(m_ffmpegPath.c_str(), 0) == -1) {
		MessageBoxA(NULL, "FFmpeg not found", "Warning", MB_OK | MB_TOPMOST);
	}

	// Look for FFprobe.exe
	std::string ffpath = m_exePath;
	ffpath += "/data/ffmpeg/ffprobe.exe";
	if (_access(ffpath.c_str(), 0) == -1) {
		MessageBoxA(NULL, "FFprobe not found", "Warning", MB_OK | MB_TOPMOST);
	}

	// Main window handle
	m_hWnd = ofGetWin32Window();

	// Set a custom window icon
	HICON hIcon = reinterpret_cast<HICON>(LoadImageA(nullptr, "bin/data/Spout.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
	SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	//
	// Create a menu using ofxWinMenu
	//

	// A new menu object with a pointer to this class
	menu = new ofxWinMenu(this, m_hWnd);
	// Register an ofApp function that is called when a menu item is selected.
	menu->CreateMenuFunction(&ofApp::appMenuFunction);
	// Create a window menu
	HMENU hMenu = menu->CreateWindowMenu();
	// File popup
	HMENU hPopup = menu->AddPopupMenu(hMenu, "File");
	// Open a movie of image file
	menu->AddPopupItem(hPopup, "Open video", false, false); // Not checked and not auto-checked
	// Explore the folder of the current movie
	menu->AddPopupItem(hPopup, "Open video folder", false, false);
	// Separator before the Exit item
	menu->AddPopupSeparator(hPopup);
	// Exit
	menu->AddPopupItem(hPopup, "Exit", false, false);
	// Output popup
	hPopup = menu->AddPopupMenu(hMenu, "Output");
	menu->AddPopupItem(hPopup, "Mute", bMute);
	menu->AddPopupItem(hPopup, "Resize", bScale);
	// Help popup
	hPopup = menu->AddPopupMenu(hMenu, "Help");
	menu->AddPopupItem(hPopup, "About", false, false); // No auto check

	// Load previous menu settings
	// after the menu items are established
	menu->Load("sender-video-audio");
	
	// Adjust window for the starting client size (in main.cpp)
	// allowing for a menu and centre on the screen
	ResetWindow();

	// Set the menu to the window after adjusting the size
	menu->SetWindowMenu();

}


//--------------------------------------------------------------
void ofApp::update() {

}


//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(0);
	ofSetColor(255);

	// If not initialized
	if (!m_pipein) {
		ofBackground(0, 20, 70); // Dark steel blue
		std::string str = "DRAG AND DROP VIDEOS HERE";
		// Get the width of the string
		int strwidth = myFont.stringWidth(str);
		// Center the string in the client area
		RECT dr ={ 0 };
		GetClientRect(m_hWnd, &dr);
		int xpos = (dr.right - dr.left)/2 - strwidth/2;
		int ypos = (dr.bottom - dr.top)/2;
		myFont.drawString(str, xpos, ypos);
		return;
	}

	// Continue to play audio if paused by
	// menu selection or click on the caption
	bNCmousePressed = false;

	// Read a video frame from the FFmpeg input pipe
	if(!bPaused) {
		if (m_pipein && m_pixelBuffer && m_SenderWidth > 0 && m_SenderHeight > 0) {
			if(fread(m_pixelBuffer, 1, m_SenderWidth * m_SenderHeight * 4, m_pipein) == 0) {
				// fread = 0 means the end of the file
				// Stop audio and draw
				bNCmousePressed = true;
				// Stop soundstream
				soundStream.stop();
				// Release resources
				CloseVideo();
				// Use the same file and pixel buffer
				m_pipein = _popen(m_input.c_str(), "rb");
				// Start the audio again as well
				m_audioPipe = _popen(m_audioInput.c_str(), "rb");
				// Start audio and draw
				bNCmousePressed = false;
				// Start soundstream
				soundStream.start();
				return;
			}
		}
		// Send the video pixels
		sender.SendImage(m_pixelBuffer, m_SenderWidth, m_SenderHeight, GL_BGRA);
	}

	// Do not draw if iconic
	if (!IsIconic(ofGetWin32Window()) && myTexture.isAllocated()) {
		// Load the draw texture with pixels
		sender.LoadTexturePixels(myTexture.getTextureData().textureID,
			myTexture.getTextureData().textureTarget,
			m_SenderWidth, m_SenderHeight, m_pixelBuffer, GL_BGRA);
		// Draw the result fitted to the display window
		myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());
		// Key shortcuts
		if(bShowInfo)
			ShowInfo();
	}

	sender.HoldFps(m_FrameRate);

}

//--------------------------------------------------------------
void ofApp::exit() {

	// Save menu settings
	menu->Save("sender-video-audio", true);
	// Release FFmpeg resources
	CloseVideo();
	// Release the sender
	if (m_pixelBuffer) delete[] m_pixelBuffer;
	m_pixelBuffer = nullptr;
	sender.ReleaseSender();

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	// Show information on-screen
	if (key == ' ') {
		bShowInfo = !bShowInfo;
	}

	// Play/Pause
	if (key == 'p' || key == 'P') {
		bPaused = !bPaused;
	}

	// Restart
	if (key == 'r' || key == 'R') {
		if (m_pipein) {
			// Stop audio and draw
			bNCmousePressed = true;
			// Stop soundstream
			soundStream.stop();
			// Release resources
			CloseVideo();
			// Use the same file and pixel buffer
			m_pipein = _popen(m_input.c_str(), "rb");
			// Start the audio again as well
			m_audioPipe = _popen(m_audioInput.c_str(), "rb");
			// Start audio and draw
			bNCmousePressed = false;
			// Cancel paused
			bPaused = false;
			soundStream.start();
		}
	}

	// Stop and close video
	if (key == 'c' || key == 'C') {
		// Stop audio and draw
		bNCmousePressed = true;
		// Stop soundstream
		soundStream.stop();
		// Release FFmpeg resources
		CloseVideo();
		// Release the sender
		if (m_pixelBuffer) delete[] m_pixelBuffer;
		m_pixelBuffer = nullptr;
		sender.ReleaseSender();
		// Start audio and draw
		bNCmousePressed = false;
		// Cancel paused
		bPaused = false;
		soundStream.start();
	}
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer &buffer)
{
	// Do not process audio for menu selection,
	// mouse click on the caption, or if paused
	if (bNCmousePressed || bPaused) {
		buffer.set(0.0f); // silence
		return;
	}

	//
	// Read the next lot of audio frames from the file.
	// This is a separate thread to Draw so the audio is not
	// limited by the video rate. The number of bytes required
	// by the audio callback and the PCM data buffer are
	// established when soundstream is set up.
	//
	if (m_audioPipe) {
		size_t bytesRead = fread(m_pcmBuffer.data(), 1, m_pcmBuffer.size()*sizeof(int16_t), m_audioPipe);
		if (bytesRead > 0) {
			// For the sound to come from the speakers
			if (!bMute) {
				size_t samplesRead = bytesRead / sizeof(int16_t);
				for (size_t i = 0; i < samplesRead; i++) {
					// A signed 16-bit sample ranges from : -32768 ... +32767
					// OpenFrameworks expects : -1.0 ... +1.0 float
					buffer[i] = static_cast<float>(m_pcmBuffer[i] / 32768.0f);
				}
				// Zero-fill any remaining samples if EOF reached
				for (size_t i = samplesRead; i < buffer.size(); i++)
					buffer[i] = 0.0f;
			}
			else {
				buffer.set(0.0f); // set silence
			}
		}
		else {
			DWORD dwError = GetLastError();
			printf("audioOut - fread failed (%d) - error = 0x%X\n", (int)bytesRead, dwError);
		}
	}
	else {
		buffer.set(0.0f);
	}
}


//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
	if (OpenVideo(dragInfo.files[0].string())) {
		OpenSender();
	}
}

//--------------------------------------------------------------
// Open FFmpeg Video and Audio pipes
bool ofApp::OpenVideo(std::string filePath)
{
	if (filePath.empty() || _access(filePath.c_str(), 0) == -1)
		return false;

	// Stop audioOut
	soundStream.stop();

	// Cancel paused
	bPaused = false;

	// Get information from the movie file using ffprobe
	// Sets the width and height globals
	if (!ffprobe(filePath)) {
		MessageBoxA(NULL, "FFprobe error", "Warning", MB_OK);
		return false;
	}

	//
	// Open an input pipe from ffmpeg
	//
	// _popen for FFmpeg and FFprobe will open a console window.
	// To hide the output, open a console first and then hide it.
	// An application can have only one console window.
	// If one exists, leave management to the application.
	// This project does not open a console window - see main.cpp
	//
	if (!GetConsoleWindow()) {
		if (AllocConsole()) {
			FILE* pCout = nullptr;
			freopen_s(&pCout, "CONOUT$", "w", stdout);
		}
		HWND hwnd = GetConsoleWindow();
		if (hwnd) {
			ShowWindow(hwnd, SW_HIDE);
			ShowWindow(hwnd, SW_MINIMIZE);
			ShowWindow(hwnd, SW_HIDE);
		}
	}

	if (m_pipein) {
		_pclose(m_pipein);
		m_pipein = nullptr;
	}
	m_input = m_ffmpegPath;
	// Quiet console output
	m_input += " -loglevel quiet";
	// Enable decoding with transparency for VP9/WebM videos.
	// Use the libvpx-vp9 codec for decoding the input.
	if (!m_codecName.empty() && m_codecName == "vp9") {
		m_input += " -c:v libvpx-vp9";
	}
	//
	m_input += " -i ";
	m_input += "\"";
	m_input += filePath;
	m_input += "\"";
	// 60 fps can be too high for FFmpeg pipe read
	// Reduce frame rate here independent of global rate
	// FFmpeg will drop frames
	double frate = m_FrameRate;
	if (frate > 30.0) {
		frate = 30.0;
		m_FrameRate = frate;
	}
	m_input += " -vf fps=";
	m_input += std::to_string(frate);
	//
	// With a constant FFmpeg frame rate, Draw is kept in sync by using
	// sleep to reach a target frame time with the HoldFps function
	//
	// FFmpeg pipe read (fread) may be too slow with 1920x1080 data
	// (typically 10-14 msec at 1920x1080, 3-4 msec at 1280x720)
	// and audio will drift out of sync. Reduce output width to 1280
	// while preserving aspect ratio
	//
	if (bScale) { // Resize menu option
		unsigned int width = 1280;
		if (m_SenderWidth > width) {
			// Calculate from m_SenderWidth/m_SenderHeight
			m_SenderHeight = width * m_SenderHeight / m_SenderWidth;
			m_SenderWidth = width;
		}
		m_input += ",scale=";
		m_input += to_string(m_SenderWidth);
		m_input += ":";
		m_input += to_string(m_SenderHeight);
	}
	// Specify BGRA pixel format to match the sender format.
	m_input += " -f image2pipe -vcodec rawvideo -pix_fmt bgra -";
	m_pipein = _popen(m_input.c_str(), "rb");
	if (m_pipein) {
		if (m_pixelBuffer) delete[] m_pixelBuffer;
		unsigned int buffersize = m_SenderWidth * m_SenderHeight * 4;
		m_pixelBuffer = new unsigned char[buffersize];
	}
	else {
		MessageBoxA(NULL, "FFmpeg open failed", "Warning", MB_OK | MB_TOPMOST);
	}

	// Audio pipe
	if (m_audioPipe) {
		_pclose(m_audioPipe);
		m_audioPipe = nullptr;
	}

	m_audioInput = m_ffmpegPath; // FFmpeg.exe path
	// Quiet console output
	m_audioInput += " -loglevel quiet";
	m_audioInput += " -i ";
	m_audioInput += "\"";
	m_audioInput += filePath; // Video file path
	m_audioInput += "\"";
	// Output raw PCM
	m_audioInput += " -f s16le";
	m_audioInput += " -acodec pcm_s16le"; // PCM signed 16-bit little-endian
	m_audioInput += " -ac ";
	m_audioInput += std::to_string(m_nChannels); // Number of channels (2 = stereo)
	m_audioInput += " -ar ";
	m_audioInput += std::to_string(m_sampleRate);  // ouput sample rate e.g. 44100 Hz
	m_audioInput += " -";
	m_audioPipe = _popen(m_audioInput.c_str(), "rb");
	m_videopath = filePath;

	if(m_pipein && m_audioPipe)
		return true;
	else
		return false;
}

//--------------------------------------------------------------
bool ofApp::OpenSender()
{
	// Stop audio and draw
	bNCmousePressed = true;
	
	sender.ReleaseSender();

	// Allocate a texture for draw
	myTexture.allocate(m_SenderWidth, m_SenderHeight, GL_RGBA);

	// Set up soundstream
	soundStream.stop(); // Stop and close for repeats
	soundStream.close();
	ofSoundStreamSettings settings;
	auto devices = soundStream.getDeviceList();
	if (!devices.empty()) {
		// Select the device number as required by the system
		settings.setOutDevice(devices[0]); // Speakers
		settings.setOutListener(this);
		settings.sampleRate = m_sampleRate;
		settings.numOutputChannels = m_nChannels;
		settings.numInputChannels = 0;
		settings.bufferSize = 1024; // Can be adjusted
		if (soundStream.setup(settings)) {
			// PCM data buffer used in audioOut
			m_pcmBuffer.resize(settings.bufferSize*settings.numOutputChannels);
			// printf("\nSoundstream setup\n");
			// printf("  nSamples     = %d\n", soundStream.getBufferSize());
			// printf("  Sample rate  = %d\n", soundStream.getSampleRate());
			// printf("  N channels   = %d\n", soundStream.getNumOutputChannels());
		}
		else {
			printf("OpenSender : Soundstream setup failed\n");
			return false;
		}
		// Allow audio and draw
		bNCmousePressed = false;
	}

	return true;

}

//--------------------------------------------------------------
// Release FFmpeg resources
void ofApp::CloseVideo()
{
	if (m_pipein) {
		// stop sound
		fflush(m_pipein);
		_pclose(m_pipein);
	}
	m_pipein = nullptr;
	if (m_audioPipe) {
		 fflush(m_audioPipe);
		_pclose(m_audioPipe);
	}
	m_audioPipe = nullptr;
}


//--------------------------------------------------------------
//
// Menu function callback
//
// This function is called by ofxWinMenu when an item is selected.
// The the title and state can be checked for required action.
// 
void ofApp::appMenuFunction(string title, bool bChecked)
{
	ofFileDialogResult result;
	string filePath;

	// Keep the audio in sync with video when menu selection
	// or mouse click on the title bar stops drawing.
	// WM_ENTERMENULOOP and WM_EXITMENULOOP are returned by ofxWinMenu
	// but are not required if WM_NCLBUTTONDOWN is tested.
	if (title == "WM_NCLBUTTONDOWN") {
		// WM_NCLBUTTONUP is not generated if the
		// mouse is released on the title bar.
		// The flag is reset when when Draw() resumes and is
		// also used when video or audio has to be stopped
		bNCmousePressed = true;
		return;
	}

	//
	// File menu
	//
	if (title == "Open video") {
		// Move to the video folder
		std::string str;
		if (!m_videopath.empty()) {
			size_t pos = m_videopath.rfind("/");
			if (pos == std::string::npos) pos = m_videopath.rfind("\\");
			str = m_videopath.substr(0, pos);
		}
		else {
			str = m_exePath;
			str += "/data/videos/";
		}
		result = ofSystemLoadDialog("Select a video file", false, str.c_str());
		if (result.bSuccess) {
			if(OpenVideo(result.filePath)) {
				OpenSender();
			}
		}
	}

	if (title == "Open video folder") {
		std::string str;
		if (!m_videopath.empty()) {
			size_t pos = m_videopath.rfind("/");
			if (pos == std::string::npos) pos = m_videopath.rfind("\\");
			str = m_videopath.substr(0, pos);
		}
		else {
			str = m_exePath;
			str += "/data/videos/";
		}

		// Does the video folder exist ?
		if (_access(str.c_str(), 0) == -1) {
			// Use the executable path as default
			str = m_exePath;
		}

		printf("%s\n", str.c_str());

		if(!ShellExecuteA(m_hWnd, "open", str.c_str(), NULL, NULL, SW_SHOWNORMAL)) {
			MessageBoxA(NULL, "No video loaded", "Warning", MB_ICONWARNING | MB_OK);
		}
	}

	if (title == "Exit") {
		ofExit();
	}

	//
	// Output menu
	//
	if (title == "Mute") {
		bMute = bChecked;
		menu->SetPopupItem("Mute", bMute);
	}

	if (title == "Resize") {
		bScale = bChecked;
		menu->SetPopupItem("Resize", bScale);
		// Close video playing
		CloseVideo();
		if (!m_videopath.empty()) {
			// Release the sender
			if (m_pixelBuffer) delete[] m_pixelBuffer;
			m_pixelBuffer = nullptr;
			sender.ReleaseSender();
			// Start again
			if(OpenVideo(m_videopath))
				OpenSender();
		}
	}

	//
	// Help menu
	//

	if (title == "About") {

		// Spout version
		std::string about = "                       Spout video sender with audio\n";
		about += "                   using Openframeworks and FFmpeg\n";
		about += "                                <a href=\"http://spout.zeal.co\">http://spout.zeal.co</a>\n";
		about += "                            Spout Version ";
		about += GetSDKversion();
		about += "\n\n";

		about += "      An example of a sender for video and audio using FFmpeg\n";
		about += "      to open two pipes, one for video and the second for audio.\n\n";
		about += "      ofSoundStream and audioOut enable sound output and Draw is\n";
		about += "      kept in sync with audio by using the HoldFps function that sleeps\n";
		about += "      to reach a target frame time. When used together with a constant\n";
		about += "      FFmpeg frame rate, the need to monitor timestamps is avoided.\n";
		about += "      This is a simple method, although seeking is not supported.\n\n";

		about += "      Uses the <a href=\"https://github.com/leadedge/ofxWinMenu\">ofxWinMenu</a> addon to create a menu and manage caption\n";
		about += "      mouse press. The source code may be useful for further reference\n\n";

		about += "      FFmpeg.exe and FFprobe.exe are required.\n";
		about += "        * Go to <a href=\"https://github.com/GyanD/codexffmpeg/releases\">https://github.com/GyanD/codexffmpeg/releases</a>\n";
		about += "        * Choose the \"Essentials\" build.\n";
		about += "          for example : ffmpeg-8.1.2-essentials_build.zip\n";
		about += "        * Download the archive and unzip to a convenient folder.\n";
		about += "        * Copy bin/FFmpeg.exe and bin/FFprobe.exe to\n";
		about += "          the application \"bin/data/ffmpeg\" folder.\n\n";

		HICON hIcon = reinterpret_cast<HICON>(LoadImageA(nullptr, "bin/data/Spout.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE));

		// Icon in the caption rather than the dialog window
		SpoutMessageBoxIconSmall();
		SpoutMessageBoxIcon(hIcon);
		SpoutMessageBoxButton(1000, L"Options");
		if (SpoutMessageBox(NULL, about.c_str(), "About", MB_USERICON | MB_OK) == 1000) {
			std::string str = "File > Open video   - Select a video file\n";
			str += "File > Video folder - open folder of the last video\n";
			str += "Output > Mute   - mute speakers\n";
			str += "Output > Resize - limit video to 1280 width (resets)\n";
			str += "  Fmpeg pipe read (fread) can be too slow with images\n";
			str += "  1920 or more width, typically 10-14 msec at 1920x1080\n";
			str += "  compared to 3-4 msec at 1280x720, and audio can drift\n";
			str += "  out sync. This option limits output width to 1280 while\n";
			str += "  preserving aspect ratio. The output frame rate is also\n";
			str += "  limited to 30fps. FFmpeg drops frames to keep that rate.\n";
			SpoutMessageBox(NULL, str.c_str(), "Options", MB_ICONINFORMATION | MB_OK);
		}

	}

} // end appMenuFunction


// Run FFprobe on a movie file and produce an ini file with the stream information
bool ofApp::ffprobe(std::string videoPath)
{
	// Get information from the movie file using ffprobe and write to an ini file
	// Use a batch file with the required ffprobe options and pass the path to ShellExecute
	std::string probepath = m_exePath;
	probepath += "/data/ffmpeg/probe.bat";

	// Does the batch file myprobe.ini exist ?
	if (_access(probepath.c_str(), 0) == -1) {
		// Create the probe.bat file
		std::ofstream batchfile(probepath);
		if (!batchfile) {
			printf("Could not create\n%s\n", probepath.c_str());
			return false;
		}
		// File created OK
		std::string str = "%~dp0/ffprobe.exe -v error -show_streams -of default=noprint_wrappers=1:nokey=1 -print_format ini -i %1 > \"%~dp0/myprobe.ini\"\n";
		batchfile << str;
		batchfile.close();
	}

	// Input to ffprobe
	std::string input = "\"";
	input += videoPath;
	input += "\"";

	// In the batch file, %~dp0 returns the Drive and Path to the batch script

	// Open ffprobe and wait for completion
	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si = { sizeof(STARTUPINFOA) };
	DWORD dwExitCode = 0;
	ZeroMemory((void *)&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; // hide the ffprobe console window
	PROCESS_INFORMATION pi{};
	std::string cmdstring = probepath + " " + input;
	if (CreateProcessA(NULL, (LPSTR)cmdstring.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		if (pi.hProcess) {
			do {
				GetExitCodeProcess(pi.hProcess, &dwExitCode);
			} while (dwExitCode == STILL_ACTIVE);
			CloseHandle(pi.hProcess);
		}
		if (pi.hThread)	CloseHandle(pi.hThread);
	}
	else {
		MessageBoxA(NULL, "FFprobe CreateProcess failed", "Warning", MB_OK | MB_TOPMOST);
		return false;
	}

	// Read the ini file produced by FFprobe to get the video information
	char initfile[MAX_PATH]{};
	strcpy_s(initfile, MAX_PATH, m_exePath.c_str());
	strcat_s(initfile, MAX_PATH, "\\DATA\\FFMPEG\\myprobe.ini");
	if (_access(initfile, 0) == -1) {
		MessageBoxA(NULL, "FFprobe ini file not found", "Warning", MB_OK | MB_TOPMOST);
		return false;
	}

	char tmp[MAX_PATH]{};
	DWORD dwResult = 0;
	m_SenderWidth = 0;
	m_SenderHeight = 0;

	// Find the first video stream
	char stream[32]{};
	for (int i=0; i<10; i++) { // arbritrary maximum
		sprintf_s(stream, 32, "streams.stream.%d", i);
		if (GetPrivateProfileStringA((LPCSTR)stream, (LPSTR)"codec_type", (LPSTR)"0", (LPSTR)tmp, 8, initfile) > 0) {
			if (strcmp(tmp, "video") == 0) {
				if (GetPrivateProfileStringA((LPCSTR)stream, (LPSTR)"width", NULL, (LPSTR)tmp, 8, initfile) > 0)
					m_SenderWidth = atoi(tmp);
				if (GetPrivateProfileStringA((LPCSTR)stream, (LPSTR)"height", NULL, (LPSTR)tmp, 8, initfile) > 0)
					m_SenderHeight = atoi(tmp);
				dwResult = GetPrivateProfileStringA((LPCSTR)"streams.stream.0", (LPSTR)"r_frame_rate", (LPSTR)"30/1", (LPSTR)tmp, 11, initfile);
				if (dwResult == 0)
					dwResult = GetPrivateProfileStringA((LPCSTR)"streams.stream.0", (LPSTR)"avm_frame_rate", (LPSTR)"30/1", (LPSTR)tmp, 11, initfile);
				if (dwResult > 0) {
					std::string iniValue = tmp;
					auto pos = iniValue.find("/");
					double num = atof(iniValue.substr(0, pos).c_str());
					double den = atof(iniValue.substr(pos + 1, iniValue.npos).c_str());
					if (num > 0.0 && den > 0.0) {
						m_FrameRate = num/den;
					}
				}
				// Video codec name
				if(GetPrivateProfileStringA((LPCSTR)"streams.stream.0", (LPSTR)"codec_name", (LPSTR)"0", (LPSTR)tmp, 20, initfile))
					m_codecName = tmp;
				break;
			}
		}
	} // end video

	// Audio
	for (int i=0; i<10; i++) { // arbritrary maximum
		sprintf_s(stream, 32, "streams.stream.%d", i);
		if (GetPrivateProfileStringA((LPCSTR)stream, (LPSTR)"codec_type", (LPSTR)"0", (LPSTR)tmp, 8, initfile) > 0) {
			if (strcmp(tmp, "audio") == 0) {
				if (GetPrivateProfileStringA((LPCSTR)stream, (LPSTR)"sample_rate", NULL, (LPSTR)tmp, 20, initfile) > 0)
					m_sampleRate = atoi(tmp);
				if (GetPrivateProfileStringA((LPCSTR)stream, (LPSTR)"channels", NULL, (LPSTR)tmp, 8, initfile) > 0)
					m_nChannels = atoi(tmp);
				break;
			}
		} // end audio
	}

	if (m_SenderWidth == 0 || m_SenderHeight == 0)
		return false;

	return true;
}

//--------------------------------------------------------------
void ofApp::ResetWindow()
{
	// Desired client size
	RECT rect{};
	GetClientRect(m_hWnd, &rect);
	float windowWidth  = (float)(rect.right - rect.left);
	float windowHeight = (float)(rect.bottom - rect.top);

	// Adjust window to desired client size allowing for the menu
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = windowWidth;
	rect.bottom = windowHeight;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_BORDER, true);

	// Full window size
	windowWidth  = (float)(rect.right - rect.left);
	windowHeight = (float)(rect.bottom - rect.top);

	// Get current position
	GetWindowRect(m_hWnd, &rect);

	// Set size and centre on the screen
	SetWindowPos(m_hWnd, NULL,
		(ofGetScreenWidth() - windowWidth)/2,
		(ofGetScreenHeight() - windowHeight)/2,
		windowWidth, windowHeight, SWP_SHOWWINDOW);

}

//--------------------------------------------------------------
void ofApp::ShowInfo()
{
	std::string str ="'P' - play/pause : 'R' - restart : 'C' - close : '  ' - show info";
	int strwidth = myFont.stringWidth(str);
	int xpos = (ofGetWidth())/2 - strwidth/2;
	int ypos = ofGetHeight()-15;
	myFont.drawString(str, xpos, ypos);
}

// ... the end
