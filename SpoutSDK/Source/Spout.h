/*

			Spout.h

			The main Spout include file for the SDK

		Copyright (c) 2014>, Lynn Jarvis. All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, 
		are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
		EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
		OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
		IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
		INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
		PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
		INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
		LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


*/
#pragma once

#ifndef __Spout__
#define __Spout__

#include "SpoutSender.h"
#include "SpoutReceiver.h"

//
//	=============   SENDER CLASS  ==============
//
//	1) create a new SpoutSender object
//
//		In your include file declare the sender object you want to create
//			SpoutSender *mySender;
//
//		In your cpp file when you want to create the sender object
//			mySender = new SpoutSender;
//
//	2) Optionally test for texture sharing compatibility of your hardware
//
//			bool GetMemoryShare();
//			bool bMemoryShareMode = myReceiver->GetMemoryShare();
//
//		Return true indicates that the harware is not texture share compatible 
//		and has or will be initialized in MemoryShare mode.
//		By using this at the start of your program, you can test for DirectX texture
//		sharing capability. This could be useful for preparative functions.
//		See below for further explanation.
//
//	3) Optionally set the DirectX 11 format of the texture to be shared
//
//			void SetDXformat(DWORD dwFormat);
//			DWORD dwFormat = (DWORD)DXGI_FORMAT_R8G8B8A8_UNORM; // For example
//			mySender->SetDXformat(dwFormat);
//
//		This sets the format of the shared DirectX texture that is created
//		for linking to your OpenGL texture and sharing by way of the NVIDIA GL/DX interop.
//		If this is not specified, a texture format compatible with DirectX 9 receivers is used 
//		(DXGI_FORMAT_B8G8R8A8_UNORM). If the hardware is not texture share compatible it will have no effect.
//
//	4) Create a named sender
//
//			bool CreateSender(char* Sendername, unsigned int width, unsigned int height);
//			bool bInitialized = mySender->CreateSender(name, width, height);
//
//		This creates a Spout sender with the given name, width and height. 
//
//		The compatibility flag will be returned true if the hardware is	texture share compatible. 
//		If the hardware is compatible you will be able to share textures by way of the NVIDIA GL/DX interop.
//		If it is not compatible you can still share textures but by shared memory instead. 
//		In this case there can only be one sender and one receiver pair	and the sender name has no effect.
//
//		There can only be one sender for this sender object. If you want more senders, simply create more
//		sender objects. 
//
//	5) When the sender changes size
//
//			bool UpdateSender(char* Sendername, unsigned int width, unsigned int height);
//			mySender->UpdateSender(Sendername, width, height);
//
//		This updates the sender with the given width and height
//
//	6) To send an OpenGL texture
//
//		bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=true);
//		mySender->SendTexture(TextureID, TextureTarget, width, height);
//
//		This sends out a texture for all receivers to detect and use. By default there is an inversion function to
//		flip the image vertically. This is needed because DirectX and OpenGL textures have the origins reversed.
//		You can bypass this by setting bInvert to false.
//
//	7)	When you want to close the sender and start again
//
//			void ReleaseSender();
//			mySender->ReleaseSender();
//
//		This closes the sender for this sender object but you can still create another sender
//		with this object once the current sender has been released. As before you can only
//		create one sender for this object.
//
//	8)	When your program is done or when you want to create a different sender object
//
//			delete mySender;
//			mySender = NULL;
//
//		This closes the object and releases all resources for the object
//
// =========================================================

//
// =====================   RECEIVER CLASS ========================
//
//	1) create a new SpoutReceiver object
//
//		In your include file declare the receiver object you want to create
//			SpoutReceiver *myReceiver;
//
//		In your cpp file when you want to create the receiver object
//			myReceiver = new SpoutReceiver;
//
//	2) Optionally test for texture sharing compatibility of your hardware
//
//			bool GetMemoryShare();
//			bool bMemoryShareMode = myReceiver->GetMemoryShare();
//
//		Return true indicates that the harware is not texture share compatible 
//		and has or will be initialized in MemoryShare mode.
//		By using this at the start of your program, you can test for DirectX texture
//		sharing capability. This could be useful for preparative functions.
//		See below for further explanation.
//
//	3) Create a receiver
//
//			bool CreateReceiver(char *Sendername, unsigned int &width, unsigned int &height);
//			bool bInitialized = myReceiver->CreateReceiver(Sendername, width, height);
//
//		This creates a Spout receiver.
//
//		The name can be specified as the name of the sender that the receiver is required to connect to.
//		If that sender is not found, the receiver connects to the "active" sender, which is the last sender
//		that the user selected, or first that was created.
//
//		The compatibility flag will be returned true if the hardware is	texture share compatible. 
//		If the hardware is compatible you will be able to share textures by way of the NVIDIA GL/DX interop.
//		If it is not compatible you can still share textures but by shared memory instead. 
//		In this case there can only be one sender and one receiver pair	and the sender name has no effect.
//
//		If a sender is found, it's name, width and height are returned.
//		If a sender is not found, the function returns false.
//		The function can be called repeatedly until a sender is found.
//
//		There can only be one receiver for this reciever object. If you want more receivers, simply create more
//		receiver objects. 
//
//	4) To receive an OpenGL texture
//
//		bool ReceiveTexture(char *Sendername, GLuint TextureID, GLuint TextureTarget, unsigned int &width, unsigned int &height);
//		bool bReceived = myReceiver->ReceiveTexture(TextureID, TextureTarget, width, height);
//
//		Pass the current width and height of your OpenGL texture.
//		Your texture will be returned with the contents updated from the sender's texture.
//
//		For success just draw the received texture.
//
//		For failure :
//
//			1) Width and height are returned zero for texture read failure.
//			   This might happen if the sender being used is closed
//
//			2) A different width and height are returned if the sender has changed size.
//			   Your local texture must then be resized.
//
//			3) A different name is returned if a different sender has been selected by the user
//			   with the "SpoutPanel" executable. 
//
//	5)	When you want to close the receiver and start again
//
//			void ReleaseReceiver();
//			mySender->ReleaseReceiver();
//
//		This closes the receiver for this receiver object but you can still create another
//		receiver with this object once the current receiver has been released. As is the case
//		with a sender, can only create one receiver for this receiver object.
//
//	7)	When your program is done or when you want to create a different receiver object
//
//			delete myReceiver;
//			myReceiver = NULL;
//
//		This closes the object and releases all resources for the object
//
//	============================== Finding senders =======================================
//
//	The receiver class has several functions for finding senders that can be used to create
//	drop down lists or menus and the like for user selection.
//
//	How many senders are running
//
//		int  GetSenderCount();
//		Returns how many senders are presently running.
//
//	Return the name of a sender in the list with a given index.
//
//		bool GetSenderName(int Index, char* Sendername, int MaxNameSize);
//
//		Index is the number of the sender in the list to retrieve. If the value of this index
//		is used again it may not retrieve the same sender name. As senders are added or removed,
//		the order of the sender names in the list will change.
//
//		Sendername is a buffer that you supply to retrieve the name of the sender.
//		MaxNameSize is the maximum number of bytes that can be returned to that buffer.
//		In any case the maximum length of a sender name is 256 bytes.
//
//		After the user has selected a sender from your drop down list or menu, release and re-create
//		a receiver using this sender name. The receiver will connect to the sender name you provide.
//
//	Return the details of a sender
//
//		bool GetSenderInfo (char* Sendername, 
//							unsigned int &Width, unsigned int &Height, 
//							HANDLE &ShareHandle, DWORD &Format);
//
//		Width and height are the dimensions of the sender texture.
//		ShareHandle is the handle of the shared DirectX 11 texture.
//		Format is the format of the shared DirectX 11 texture.
//
//		You may wish to find the width and height of a user selected sender before
//		creating a receiver, so that you can prepare your OpenGL texture. The receiver
//		will then connect to and recieve a texture from the sender straight away.
//
//	Set a sender as the "active" sender.
//
//		bool SetActiveSender(char* Sendername);
//
//		An active sender will be detected by any receiver that is Created.
//		If you want to control this function, you can set any of the current sender as "active".
//		Typically the active sender will be the last one that the user has selected.
//		You can preserve this concept by setting any sender that the user selects from your
//		drop down list or menu as the active sender.
//
//	Finding the "active" sender
//
//		bool GetActiveSender(char* Sendername);
//
//		This will return either the first sender started or the last one selected by the
//		user using "SpoutPanel" as below or by using the "SpoutTray" taskbar utility.
//		For example, if your aplication does not provide any means for user selection
//		from a list of senders, they can use "SpoutTray" to select one. Then your program
//		can provide a more simple cotrol such as a button to switch to the active sender.
//		A receiver created using this name will connect and return the sender dimensions.
//
//	Select a sender
//
//		bool SelectSenderDialog();
//		bool SelectSenderPanel();
//
//		SelectSenderDialog() activates a modal dialog and SelectSenderPanel() activates
//		an executable program "SpoutPanel.exe" which allow the user	to select a sender.
//
//		Return true means that the user has selected a sender.
//		The selected sender is then set as the "active" sender. 
//
//		If a receiver is currently running and receiving textures, it will be re-initialized
//		internally and switch to the active sender the next time "ReceiveTexture" is called.
//		The the new name, width and height will be returned.
//
//		Note that SelectSenderDialog requires "spoutResource.rc" and "spotResouce.h" to
//		be included in your program project. Because SelectSenderDialog() is a modal dialog 
//		it will interrupt Windows messages and in some cases this can cause problems. 
//		If so, use SelectSenderPanel instead. To be independent of either of these, use
//		the sender finding methods above.
//
// ======================= COMMON FUNCTIONS ===========================
//
//		bool GetMemoryShareMode()
//
//		Return whether the hardware capabilities only allow initialization in
//		Memory Sharing mode mode.
//
//		bool SetMemoryShareMode()
//
//		Set Memory Sharing mode so that hardware compatibility is not tested
//		during intialization and memory sharing is used even if the
//		hardware is texture share compatible.
//


#endif
