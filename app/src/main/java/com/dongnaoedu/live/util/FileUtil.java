package com.dongnaoedu.live.util;

import android.graphics.Bitmap;
import android.os.Environment;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class FileUtil {
    private static final String TAG = "xcb_FileUtil";

    public static void writeBytesToFile(String fileName, byte[] data){
        InputStream is = null;
        OutputStream out = null;
        try {
            out = new FileOutputStream(fileName, true);
            is = new ByteArrayInputStream(data);
            byte[] buff = new byte[1024];
            int len = 0;
            while((len=is.read(buff))!=-1){
                out.write(buff, 0, len);
            }

        } catch (IOException e) {
            Log.e(TAG, e.toString(), e);
        } finally {
            try {
                if (is != null) {
                    is.close();
                }
            } catch (IOException e) {
                Log.e(TAG, e.toString(), e);
            }
            try {
                if (out != null) {
                    out.close();
                }
            } catch (IOException e) {
                Log.e(TAG, e.toString(), e);
            }
        }
    }

    public static String fileName = "meal_log";
    private static String filePath = Environment.getExternalStorageDirectory() + File.separator + fileName + File.separator;
    public static void saveBitmapAsPng(Bitmap bitmap, String picName) {
        // 2. 保存Bitmap到文件
        File mediaStorageDir = new File(filePath);
        if (!mediaStorageDir.exists()) {
            mediaStorageDir.mkdirs();
        }
        try (FileOutputStream fos = new FileOutputStream(filePath + picName)) {
            // 3. 将Bitmap压缩为PNG格式并写入文件
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, fos);
        } catch (IOException e) {
            Log.e(TAG, "saveBitmapAsPng：：" + e.toString(), e);
        } finally {
            // 4. 回收Bitmap资源
            bitmap.recycle();
        }
    }
}
