package com.github.dzli.unlockproxy;

import java.io.*;
import android.app.Activity;
import android.os.Bundle;
import android.app.AlertDialog;
import android.view.View;
import android.content.Context;
import android.content.DialogInterface;
import android.widget.Switch;
import android.widget.CheckBox;
import android.content.SharedPreferences;

public class MainActivity extends Activity
{
    static final String DATA_DIR = "/data/data/com.github.dzli.unlockproxy";
    final Context context = this;

    /** Called when the activity is first created. */
    @Override
        public void onCreate(Bundle savedInstanceState)
        {   
            super.onCreate(savedInstanceState);
            setContentView(R.layout.main);
            copyFilesInAssets("hproxy", DATA_DIR + "/hproxy");
            chgFilePermission(DATA_DIR + "/hproxy", "755");
            copyFilesInAssets("hproxy.conf", DATA_DIR + "/hproxy.conf");

            SharedPreferences sharedPref = getMyPreferences();
            boolean autostartValue = sharedPref.getBoolean("EnableAutoStart", false);
            CheckBox autostartCheckBox = (CheckBox) findViewById(R.id.checkbox_autostart);
            autostartCheckBox.setChecked(autostartValue);

            Switch s = (Switch) findViewById(R.id.switch_proxy);
            s.setChecked(proxyIsRunning());
        }   

    private SharedPreferences getMyPreferences()
    {
        return getSharedPreferences("hproxy", Context.MODE_PRIVATE);
    }

    private boolean proxyIsRunning(){
        String pidFilePath = "/data/local/tmp/hproxy.pid"; 
        java.io.File file = new java.io.File(pidFilePath);
        return file.exists();
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

    private void doStartProxy()
    {
        try {
            Process p =  Runtime.getRuntime().exec("su");
            OutputStream os = p.getOutputStream();
            DataOutputStream dos = new DataOutputStream(os);
            dos.writeBytes(DATA_DIR+"/hproxy start -c "+DATA_DIR+"/hproxy.conf\n");
            dos.flush();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void doStopProxy()
    {
        try {
            Process p =  Runtime.getRuntime().exec("su");
            OutputStream os = p.getOutputStream();
            DataOutputStream dos = new DataOutputStream(os);
            dos.writeBytes(DATA_DIR+"/hproxy stop -c "+DATA_DIR+"/hproxy.conf\n");
            dos.flush();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void testStartProxy()
    {   
        String cmd = "/system/bin/ls >"  + DATA_DIR +"/output.txt";
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

    public void onToggleClicked(View view) 
    {
        boolean on = ((Switch) view).isChecked();

        if (on) {
            doStartProxy();
        } else {
            doStopProxy();
        }
    }

    public void onCheckboxClicked(View view) 
    {
        boolean checked = ((CheckBox) view).isChecked();

        switch(view.getId()) {
            case R.id.checkbox_autostart:
                SharedPreferences sharedPref = getMyPreferences();
                SharedPreferences.Editor editor = sharedPref.edit();
                editor.putBoolean("EnableAutoStart", checked);
                editor.commit();
                break;
        }
    }

}

