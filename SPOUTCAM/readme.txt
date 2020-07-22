SPOUTCAM

SpoutCam is a virtual webcam that is also a Spout receiver.

SPOUTCAM SETTINGS

Some host programs don't accept arbitrary resolutions or 60 fps frame rate, and expect typical webcam output. "SpoutCamSettings" allows the frame rate and resolution to be specified and SpoutCam will use these settings when it starts.

Defaults are the same as previous versions of SpoutCam - "Active sender" and "60fps". If you are having trouble with a host program, select "640x480" and "30fps" so that SpoutCam behaves more like a typical webcam.

RESOLUTION

The resolution depends on :

  o What has been selected by SpoutCamSettings
  o Whether a sender is already running.

1) If a sender is running when SpoutCam starts, it will adapt to the sender resolution if "Active sender" has been selected as the starting resolution.

2) If any other fixed resolution has been selected, it will always be used whether the sender is started first or not.

3) If no sender is running when SpoutCam starts and "Active sender" has been selected, the resolution is undefined, so the default 640x480 is used.

FRAME RATE

You can use 60fps if it is accepted by the host program and if the CPU performance is sufficient for the processing required. Use 30fps for best compatibility with webcams. The frame rate selected is the “desired” frame rate. SpoutCam will attempt to meet that frame rate and drop frames if it cannot keep up.

REGISTER

If an installer has been used there is an option to register SpoutCam during the installation. 
For a portable installation, Click the REGISTER button.
Both these installations will register both 32 bit and 64 bit versions of SpoutCam.

You can also register it manually. There are folders for 32 bit and 64 bit versions and you can install both on a 64 bit system. In each folder you will find "_register_run_as_admin.bat". RH click and "Run as Administrator" to register SpoutCam. 

Not all applications support virtual webcams and SpoutCam might not show up for some of them. If you have problems, you can un-register SpoutCam and Register again at any time.
