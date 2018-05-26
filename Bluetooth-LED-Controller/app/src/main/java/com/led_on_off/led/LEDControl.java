package com.led_on_off.led;

import android.app.Fragment;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.AsyncTask;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import java.io.IOException;
import java.util.UUID;

import static java.lang.Boolean.FALSE;

/**
 * Created by Rut Vora
 */


public class LEDControl extends Fragment implements View.OnClickListener, TextWatcher, SeekBar.OnSeekBarChangeListener {

    //SPP UUID. Look for it
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    // Button btnOn, btnOff, btnDis;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    SeekBar redSeekBar;
    Spinner redSpinner;
    TextView redValue;
    SeekBar greenSeekBar;
    Spinner greenSpinner;
    TextView greenValue;
    SeekBar blueSeekBar;
    Spinner blueSpinner;
    TextView blueValue;
    boolean userMode = FALSE;
    private String address = null;
    private ProgressDialog progress;
    private boolean isBtConnected = false;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_led_control, container, false);

        redSeekBar = rootView.findViewById(R.id.seekBarRed);
        redSpinner = rootView.findViewById(R.id.spinnerRed);
        redValue = rootView.findViewById(R.id.editRed);
        //redValue.addTextChangedListener(this);
        redSeekBar.setOnSeekBarChangeListener(this);

        blueSeekBar = rootView.findViewById(R.id.seekBarBlue);
        blueSpinner = rootView.findViewById(R.id.spinnerBlue);
        blueValue = rootView.findViewById(R.id.editBlue);
        //blueValue.addTextChangedListener(this);
        blueSeekBar.setOnSeekBarChangeListener(this);

        greenSeekBar = rootView.findViewById(R.id.seekBarGreen);
        greenSpinner = rootView.findViewById(R.id.spinnerGreen);
        greenValue = rootView.findViewById(R.id.editGreen);
        //greenValue.addTextChangedListener(this);
        greenSeekBar.setOnSeekBarChangeListener(this);

        ToggleButton userToggle = rootView.findViewById(R.id.userToggle);
        //userToggle.isChecked()

        Button disconnect = rootView.findViewById(R.id.disconnect);
        disconnect.setOnClickListener(this);

        Button update = rootView.findViewById(R.id.update);
        update.setOnClickListener(this);

        new ConnectBT().execute();
        return rootView;
    }

    private void disconnect() {
        if (btSocket != null) //If the btSocket is busy
        {
            try {
                btSocket.close(); //close connection
            } catch (IOException e) {
                Toast.makeText(getActivity(), "Error", Toast.LENGTH_SHORT).show();
            }
            getFragmentManager().popBackStack();

        }
    }

    private void update(String message) {
        if (btSocket != null) {
            try {
                btSocket.getOutputStream().write(message.getBytes());
            } catch (IOException e) {
                Toast.makeText(getActivity(), "Error", Toast.LENGTH_SHORT).show();
            }
        }
    }

    protected void setAddress(String address) {
        this.address = address;
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.update) {
            String msg = "#R" + redValue.getText().toString() + "G" + greenValue.getText().toString() + "B" + blueValue.getText().toString() + "~";
            update(msg);
            //TODO
        } else if (view.getId() == R.id.disconnect) {
            disconnect();
        }
    }

    @Override
    public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {
        //Nothing
    }

    @Override
    public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {
        //Nothing
    }

    @Override
    public void afterTextChanged(Editable editable) {
        try {
            if (Integer.parseInt(editable.toString()) <= 255 && Integer.parseInt(editable.toString()) >= 0) {
                View changed = getActivity().getCurrentFocus();
                if (changed.getTag() == null) {
                    if (changed.getId() == R.id.editRed) {
                        redSeekBar.setTag("Bla");
                        redSeekBar.setProgress(Integer.parseInt(editable.toString()));
                        redSeekBar.setTag(null);
                    }
                    if (changed.getId() == R.id.editGreen) {
                        greenSeekBar.setTag("Bla");
                        greenSeekBar.setProgress(Integer.parseInt(editable.toString()));
                        greenSeekBar.setTag(null);
                    }
                    if (changed.getId() == R.id.editBlue) {
                        blueSeekBar.setTag("Bla");
                        blueSeekBar.setProgress(Integer.parseInt(editable.toString()));
                        blueSeekBar.setTag(null);
                    }
                }
            }
        } catch (Exception e) {
            Toast.makeText(getActivity(), "Only numbers allowed!", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
        if (b) {
            if (seekBar.getTag() == null) {
                if (seekBar.getId() == R.id.seekBarRed) {
                    redValue.setTag("Bla");
                    redValue.setText(i + "");
                    redValue.setTag(null);
                }
                if (seekBar.getId() == R.id.seekBarGreen) {
                    greenValue.setTag("Bla");
                    greenValue.setText(i + "");
                    greenValue.setTag(null);
                }
                if (seekBar.getId() == R.id.seekBarBlue) {
                    blueValue.setTag("Bla");
                    blueValue.setText(i + "");
                    blueValue.setTag(null);
                }
            }
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        //Do Nothing
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        //Do Nothing
    }


    private class ConnectBT extends AsyncTask<Void, Void, Void>  // UI thread
    {
        private boolean ConnectSuccess = true; //if it's here, it's almost connected

        @Override
        protected void onPreExecute() {
            progress = ProgressDialog.show(getActivity(), "Connecting...", "Please wait!!!");  //show a progress dialog
        }

        @Override
        protected Void doInBackground(Void... devices) //while the progress dialog is shown, the connection is done in background
        {
            try {
                if (btSocket == null || !isBtConnected) {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device
                    BluetoothDevice device = myBluetooth.getRemoteDevice(address);//connects to the device's address and checks if it's available
                    btSocket = device.createInsecureRfcommSocketToServiceRecord(myUUID);//create a RFCOMM (SPP) connection
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();//start connection
                }
            } catch (IOException e) {
                ConnectSuccess = false;//if the try failed, you can check the exception here
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) //after the doInBackground, it checks if everything went fine
        {
            super.onPostExecute(result);

            if (!ConnectSuccess) {
                Toast.makeText(getActivity(), "Connection Failed. Is it a SPP Bluetooth? Try again.", Toast.LENGTH_SHORT).show();
                getFragmentManager().popBackStack();

            } else {
                Toast.makeText(getActivity(), "Connected", Toast.LENGTH_SHORT).show();
                isBtConnected = true;
            }
            progress.dismiss();
        }
    }
}
