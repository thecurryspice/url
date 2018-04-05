package com.led_on_off.led;

import android.app.Activity;
import android.os.Bundle;

/**
 * Created by Rut Vora
 */
public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getFragmentManager().beginTransaction().replace(R.id.fragment, new DeviceList()).commit();
    }

}
