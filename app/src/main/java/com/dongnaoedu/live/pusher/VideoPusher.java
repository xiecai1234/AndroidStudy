package com.dongnaoedu.live.pusher;

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;

import com.dongnaoedu.live.Nv21ToBitmap;
import com.dongnaoedu.live.jni.PushNative;
import com.dongnaoedu.live.params.VideoParam;
import com.dongnaoedu.live.util.BitmapUtil;
import com.dongnaoedu.live.util.Const;
import com.dongnaoedu.live.util.FileUtil;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.PreviewCallback;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;

public class VideoPusher extends Pusher implements Callback, PreviewCallback{
	private static final String TAG = "xcb_VideoPusher";
	private SurfaceHolder surfaceHolder;
	private Camera mCamera;
	private VideoParam videoParams;
	private byte[] buffers;
	private boolean isPushing = false;
	private PushNative pushNative;
	private Nv21ToBitmap nv21ToBitmap;

	public VideoPusher(Context context, SurfaceHolder surfaceHolder, VideoParam videoParams, PushNative pushNative) {
		this.surfaceHolder = surfaceHolder;
		this.videoParams = videoParams;
		this.pushNative = pushNative;
		surfaceHolder.addCallback(this);
		nv21ToBitmap = new Nv21ToBitmap(context);
	}

	@Override
	public void startPush() {
		Log.i(TAG, "startPush");
		//设置视频参数
		pushNative.setVideoOptions(videoParams.getWidth(), 
				videoParams.getHeight(), videoParams.getBitrate(), videoParams.getFps());
		isPushing = true;
	}

	@Override
	public void stopPush() {
		Log.i(TAG, "stopPush");
		isPushing = false;
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.i(TAG, "surfaceCreated");
		startPreview();
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {

	}
	
	@Override
	public void release() {
		stopPreview();
	}


	/**
	 * 切换摄像头
	 */
	public void switchCamera() {
		Log.i(TAG, "switchCamera");
		if(videoParams.getCameraId() == CameraInfo.CAMERA_FACING_BACK){
			videoParams.setCameraId(CameraInfo.CAMERA_FACING_FRONT);
		}else{
			videoParams.setCameraId(CameraInfo.CAMERA_FACING_BACK);
		}
		//重新预览
		stopPreview();
		startPreview();
	}
	
	/**
	 * 开始预览
	 */
	private void startPreview() {
		Log.i(TAG, "startPreview");
		try {
			//SurfaceView初始化完成，开始相机预览
			mCamera = Camera.open(videoParams.getCameraId());
			Camera.Parameters parameters = mCamera.getParameters();
			//设置相机参数
			//预览图像的像素格式,属于YUV格式，跟YUV420P的区别是U和V位置调换了
			parameters.setPreviewFormat(ImageFormat.NV21);
			// 因为parameters.setPictureSize(320, 480)(设置分辨率)的参数有误，如果不清楚分辨率可以却掉这句话，再运行就OK了。 但是会变形
//			parameters.setPreviewSize(videoParams.getWidth(), videoParams.getHeight()); //预览画面宽高

			// 设置相机分辨率
			List<Camera.Size> supportedSizes = parameters.getSupportedPreviewSizes();
			Camera.Size optimalSize = getOptimalPreviewSize(supportedSizes, videoParams.getWidth(),
					videoParams.getHeight());
			if (optimalSize != null) {
				parameters.setPreviewSize(optimalSize.width, optimalSize.height);
			}
			// 设置对焦模式
			List<String> supportedFocusModes = parameters.getSupportedFocusModes();
			if (supportedFocusModes.contains(Camera.Parameters.FOCUS_MODE_AUTO)) {
				parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
			}
			mCamera.setParameters(parameters);
			//parameters.setPreviewFpsRange(videoParams.getFps()-1, videoParams.getFps());
			mCamera.setPreviewDisplay(surfaceHolder);
			// 旋转为竖屏预览画面显示才正常，但数据还是横屏的
			mCamera.setDisplayOrientation(90);
			//获取预览图像数据 根据addCallbackBuffer方法的注释计算大小
			float bytesPerPixel = ImageFormat.getBitsPerPixel(ImageFormat.NV21) / 8.0f;
//			int size = (int) (videoParams.getWidth() * videoParams.getHeight() * bytesPerPixel) + 1;
			// 取大一点没关系，能装下就行
			int size = (int) (videoParams.getWidth() * videoParams.getHeight() * 2) ;
			buffers = new byte[size];
			Log.i(TAG, "buffers.length:" + buffers.length);
			mCamera.addCallbackBuffer(buffers);
			mCamera.setPreviewCallbackWithBuffer(this);
			mCamera.startPreview();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * 停止预览
	 */
	private void stopPreview() {
		Log.i(TAG, "stopPreview");
		if(mCamera != null){			
			mCamera.stopPreview();
			mCamera.release();
			mCamera = null;
		}
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		// onPreviewFrame 不回调的原因buffers分配的太小 ，Callback buffer was too small! Expected 3110400 bytes, but got 614400 bytes!
		Camera.Size size = camera.getParameters().getPreviewSize();
		//onPreviewFrame,width:1920,height:1080
		Log.i(TAG, "onPreviewFrame,width:" + size.width + ",height:" + size.height + "data.length:" + data.length);
		//TODO 待取消注释 注释这段下次就不会再回调onPreviewFrame
		// TODO 避免写入过多数据
//		if(mCamera != null){
//			mCamera.addCallbackBuffer(buffers);
//		}
		FileUtil.writeBytesToFile(Const.NAME, data);

//		saveData2(data, size);
		if(isPushing){
			//回调函数中获取图像数据，然后给Native代码编码
			pushNative.fireVideo(data);
		}
	}

	private void saveData(byte[] data, Camera.Size size) {
		Bitmap bitmap = nv21ToBitmap.nv21ToBitmap(data, size.width, size.height);
		// 8294400 = 1080 * 1920 * 4  每个像素4字节  对应Bitmap.Config.ARGB_8888 每个通道8位，共32位，即4个字节
		Log.e(TAG, "保存图片 bitmap= " + bitmap.getByteCount());
		//旋转图片
		Bitmap rotateBitmap = BitmapUtil.rotateBitmap(90, bitmap);
		Log.e(TAG, "旋转后图片 bitmap= " + rotateBitmap.getByteCount());
		FileUtil.saveBitmapAsPng(rotateBitmap, "palm_detect_test.png");
	}

	// jpg比另一个保存saveData方法压缩更多
	private void saveData2(byte[] data, Camera.Size size) {
		try {
			YuvImage image = new YuvImage(data, ImageFormat.NV21, size.width, size.height, null);
			ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
			image.compressToJpeg(new Rect(0, 0, size.width, size.height), 100, outputStream);
			byte[] imageBytes = outputStream.toByteArray();

			FileOutputStream fileOutputStream = new FileOutputStream("/sdcard/camera_test.jpg");
			fileOutputStream.write(imageBytes);
			fileOutputStream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	// 获取最佳预览尺寸
	private Camera.Size getOptimalPreviewSize(List<Camera.Size> sizes, int width, int height) {
		final double ASPECT_TOLERANCE = 0.1;
		double targetRatio = (double) width / height;
		Camera.Size optimalSize = null;
		double minDiff = Double.MAX_VALUE;
		// 遍历支持的预览尺寸，选择与目标宽高比最接近的尺寸
		for (Camera.Size size : sizes) {
			double ratio = (double) size.width / size.height;
			if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE)
				continue;
			if (Math.abs(size.height - height) < minDiff) {
				optimalSize = size;
				minDiff = Math.abs(size.height - height);
			}
		}
		// 如果没有找到与目标宽高比最接近的尺寸，选择高度最接近的尺寸
		if (optimalSize == null) {
			minDiff = Double.MAX_VALUE;
			for (Camera.Size size : sizes) {
				if (Math.abs(size.height - height) < minDiff) {
					optimalSize = size;
					minDiff = Math.abs(size.height - height);
				}
			}
		}
		return optimalSize;
	}

}
