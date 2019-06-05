SPOUTCAM

SpoutCam is a virtual webcam that is also a Spout receiver.

Not all applications support virtual webcams so if you have problems, uninstall Spout and re-install without the SpoutCam option selected.

If SpoutCam is registered by the installer, there is no need to take any further action.

However, if there has been no installation, you will be prompted to register SpoutCam when SpoutCamSettings is opened. 

You can also register it manually. There are folders for 32 bit and 64 bit versions and you can install both on a 64 bit system. 
In each folder you will find "_register_run_as_admin.bat". 
RH click and "Run as Administrator" to register SpoutCam. 


SPOUTCAM SETTINGS

Previous versions of SpoutCam have always used use the resolution of the first sender running when it starts. But some host programs expect typical webcam formats such as 640x480 and a frame rate of 30 fps instead of the default 60fps.

"SpoutCamSettings.exe" allows the frame rate and resolution for SpoutCam to be specified.

For example if you are having trouble with a host program, it could be better to select "640x480" and "30fps" so that SpoutCam behaves more like a typical webcam.

RESOLUTION

The resolution actually used depends on :

  o What has been selected by SpoutCamSettings
  o Whether a sender is already running.

1) If a sender is running when SpoutCam starts, it will adapt to the sender resolution if "Active sender" has been selected as the starting resolution.

2) If any other fixed resolution has been selected, it will always be used whether the sender is started first or not.

3) If no sender is running when SpoutCam starts and "Active sender" has been selected, the resolution is undefined, so the default 640x480 is used.

FRAME RATE

You will get better performance by selecting 60fps if the host program accepts it, and the CPU performance is sufficient for the processing required.

The frame rate set is the “desired” frame rate. SpoutCam will attempt to meet that frame rate and drop frames if it cannot keep up.

Some applications will not accept 60fps and might require 30fps to be selected.

REGISTER

If SpoutCam has not been registered you can register with this button. 
If it has already been registered you have the option to un-register it. 

SUMMARY

For SpoutCam to adapt to the resolution of the sender, select "Active sender" in the settings and start the sender before starting SpoutCam. This is the most efficient way to operate because there is no image size conversion.

If you want to use common webcam resolutions for compatibility reasons, select the resolution required and SpoutCam will retain that resolution.

Use 30fps for best compatibility with webcams. 60fps might give better performance if it is accepted by the host program.





