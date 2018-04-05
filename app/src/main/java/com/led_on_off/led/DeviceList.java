package com.led_on_off.led;

import android.app.Fragment;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Set;

import static android.app.Activity.RESULT_OK;

/**
 * Created by Rut Vora
 */

public class DeviceList extends Fragment implements AdapterView.OnItemClickListener {
    public static String address = "device_address";
    //widgets
    ListView devicelist;
    ArrayAdapter<String> adapter;
    //Bluetooth
    private BluetoothAdapter myBluetooth = null;
    private ArrayList<BluetoothDevice> devices = new ArrayList<>();
    private ArrayList<String> list = new ArrayList<>();
    /*private AdapterView.OnItemClickListener myListClickListener = new AdapterView.OnItemClickListener() {
        public void onItemClick(AdapterView<?> av, View v, int arg2, long arg3) {
            // Get the device MAC address, the last 17 chars in the View
            String info = ((TextView) v).getText().toString();
            String address = info.substring(info.length() - 17);

            // Make an intent to start next activity.
            //Intent i = new Intent(DeviceList.this, LEDControl.class);

            //Change the activity.
            i.putExtra(EXTRA_ADDRESS, address); //this will be received at LEDControl (class) Activity
            startActivity(i);
        }
    };
    */

    BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                //TODO
                Log.w("Discovery Finished", "Yes");

            } else if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                //Add the device to the list of available devices
                BluetoothDevice bt = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (!devices.contains(bt)) {
                    devices.add(bt);
                    String newBTDevice = bt.getName() + "\n" + bt.getAddress();
                    list.add(newBTDevice); //Get the device's name and the address
                    adapter.notifyDataSetChanged();
                }
            } else if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(action)) {
                if (intent.getParcelableExtra(BluetoothDevice.EXTRA_BOND_STATE).toString().equals(BluetoothDevice.BOND_BONDED + "")) {
                    //TODO Send Intent to LEDControl
                    LEDControl ledControl = new LEDControl();
                    ledControl.setAddress(address);
                    getFragmentManager().beginTransaction().replace(R.id.fragment, ledControl).addToBackStack(null).commit();
                }
            }
        }
    };

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_device_list, container, false);

        //Calling widgets
        //btnPaired = (Button)findViewById(R.id.button);
        devicelist = rootView.findViewById(R.id.listView);

        //if the device has bluetooth
        myBluetooth = BluetoothAdapter.getDefaultAdapter();

        if (myBluetooth == null) {
            //Show a message. that the device has no bluetooth adapter
            Toast.makeText(getActivity(), "Bluetooth Device Not Available", Toast.LENGTH_LONG).show();

            //finish apk
            getActivity().finish();
        } else if (!myBluetooth.isEnabled()) {
            //Ask to the user turn the bluetooth on
            Intent turnBTon = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnBTon, 1);
        } else if (myBluetooth.isEnabled()) getDevicesList();

        //Intent Filters to receive only the intents we want
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothDevice.ACTION_FOUND);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);

        //Register the broadcast receiver
        getActivity().registerReceiver(mReceiver, filter);

        return rootView;

    }

    private void getDevicesList() {
        Set<BluetoothDevice> pairedDevices = myBluetooth.getBondedDevices();
        Log.w("Paired Devices: ", pairedDevices.size() + "");
        BluetoothDevice[] paired = new BluetoothDevice[pairedDevices.size()];
        pairedDevices.toArray(paired);
        for (int i = 0; i < paired.length; i++) {
            BluetoothDevice bt = paired[i];
            Log.w("String for " + i, bt.toString() + " and " + bt.getName());
            if (!devices.contains(bt)) {
                devices.add(bt);
                list.add(bt.getName() + "\n" + bt.getAddress()); //Get the device's name and the address
            }
        }

        adapter = new ArrayAdapter<>(getActivity(), android.R.layout.simple_list_item_1, list);
        devicelist.setOnItemClickListener(this); //Method called when the device from the list is clicked
        devicelist.setAdapter(adapter);
        myBluetooth.startDiscovery();

    }

    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        TextView device = (TextView) view;
        String info = device.getText().toString();
        address = info.substring(info.length() - 17);
        if (devices.get(i).getBondState() == BluetoothDevice.BOND_NONE) {
            devices.get(i).createBond();
        } else if (devices.get(i).getBondState() == BluetoothDevice.BOND_BONDED) {
            //TODO Send intent to LEDControl
            LEDControl ledControl = new LEDControl();
            ledControl.setAddress(address);
            getFragmentManager().beginTransaction().replace(R.id.fragment, ledControl).addToBackStack(null).commit();
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.w("OnActivityResult: ", requestCode + "");
        if (requestCode == 1) {
            if (resultCode == RESULT_OK) {
                Log.w("Bluetooth Turned On", "No issues");
                getDevicesList();
            } else {
                Toast.makeText(getActivity(), "Please Switch on Bluetooth", Toast.LENGTH_SHORT).show();
                Intent turnBTon = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(turnBTon, 1);
            }
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        getActivity().unregisterReceiver(mReceiver);
    }
}
