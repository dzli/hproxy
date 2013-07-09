package com.github.dzli.unlockproxy;

import java.io.*;
import android.app.Activity;
import android.os.Bundle;

public class MainActivity extends Activity
{
    static final String DATA_DIR = "/data/data/com.example.myfirstapp";

    /** Called when the activity is first created. */
    @Override
        public void onCreate(Bundle savedInstanceState)
        {   
            super.onCreate(savedInstanceState);
            setContentView(R.layout.main);
            copyFilesInAssets("htest", DATA_DIR + "/htest");
            chgFilePermission(DATA_DIR + "/htest", "755");
        }   

    private void copyFilesInAssets(String fileName, String toPath)
    {   
        try 
        {   
            InputStream myInput = getAssets().open(fileName);
            String outFileName = toPath;
            OutputStream myOutput = new FileOutputStream(outFileName);
            byte[] buffer = new byte[1024];
            int length;
            while ((length = myInput.read(buffer))>0)
            {   
                myOutput.write(buffer, 0, length);
            }   

            myOutput.flush();
            myOutput.close();
            myInput.close();
        }   
        catch(Exception ex) 
        {   
            ex.printStackTrace();
        }   
    }   

    private void chgFilePermission(String filePath, String permission)
    {   
        String cmd = "/system/bin/chmod " + permission + " " + filePath;
        String[] strExec = new String[] 
        {   
            "/system/bin/sh", "-c", cmd 
        };  

        try {
            Runtime.getRuntime().exec(strExec);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }   
    }   
}

