package com.github.dzli.unlockproxy;

import java.io.*;
import android.content.BroadcastReceiver;
import android.os.Bundle;
import android.content.Context;
import android.content.Intent;

public class BootReceiver extends BroadcastReceiver
{
    static final String DATA_DIR = "/data/data/com.github.dzli.unlockproxy";

    @Override
    public void onReceive(Context context, Intent intent) {
        doStartProxy();
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


}

