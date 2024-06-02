package com.dongnaoedu.live.util;

import android.graphics.Bitmap;
import android.graphics.Matrix;

public class BitmapUtil {
    public static Bitmap rotateBitmap(int angle, Bitmap bitmap){
        if(bitmap == null){
            return null;
        }
        Bitmap returnBm = null;
        //根据旋转角度，生成旋转矩阵
        Matrix matrix = new Matrix();
        matrix.postRotate(angle);
        try {
            //将原始图片按照旋转矩阵进行旋转，并得到新的图片
            returnBm = Bitmap.createBitmap(bitmap,  0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
        } catch (Exception exception) {
            exception.printStackTrace();
        }
        if(returnBm == null){
            returnBm = bitmap;
        }
        return returnBm;
    }
}
