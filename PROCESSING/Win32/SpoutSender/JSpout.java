class JSpout {

  static {  
    System.loadLibrary("JSpout");
  }
 
  
  static native int InitSender(String name, int width, int height, int mode);

  static native int InitReceiver(String name, int[] dim, int mode);

  static native boolean ReleaseSender();

  static native boolean ReleaseReceiver();

  static native boolean WriteToSharedMemory(int w, int h, int[] pix);

  static native boolean ReadFromSharedMemory(int[] dim, int[] pix);

  static native boolean ReadTexture(int[] dim, int[] pix);

  static native boolean WriteTexture(int w, int h, int texID, int texTarget, boolean bInvert);

  static native String GetSenderName();
  
  static native boolean SenderDialog();

  static native boolean CreateControl(String name, String type, float minimum, float maximum, float value, String text);
  
  static native boolean OpenControls(String name);

  static native int CheckControls(String[] name, int[] type, float[] value, String[] text);

  static native boolean CloseControls();


}
