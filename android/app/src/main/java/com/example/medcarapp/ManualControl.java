package com.example.medcarapp;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttToken;

import io.github.controlwear.virtual.joystick.android.JoystickView;
import mqttController.CarConnect;
import mqttController.MqttClient;

public class ManualControl extends AppCompatActivity {
    // joystick adapted from: https://github.com/controlwear/virtual-joystick-android
    private static final int QOS = 1;
    private static final String TURNING_TOPIC = "/smartcar/control/turning";
    private static final String SPEED_TOPIC = "/smartcar/control/speed";
    CarConnect carConnect;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_manual_control);

        carConnect = new CarConnect(getApplicationContext());
        carConnect.connectToMqttBroker();

        JoystickView joystick = (JoystickView) findViewById(R.id.joystickView2);

        joystick.setOnMoveListener(new JoystickView.OnMoveListener() {
            int prevAngle = -10000;
            int prevSpeed = -10000;
            @SuppressLint("ClickableViewAccessibility")
            @Override
            public void onMove(int angle, int strength) {
                int adjustedAngle = adjustAngle(angle);
                int adjustedSpeed = adjustSpeed(strength, angle);
                if (adjustedAngle != prevAngle){
                    carConnect.publish(TURNING_TOPIC, Integer.toString(adjustedAngle), QOS, null);
                }
                if (adjustedSpeed != prevSpeed){
                    carConnect.publish(SPEED_TOPIC, Integer.toString(adjustedSpeed), QOS, null);
                }
                prevAngle = adjustedAngle;
                prevSpeed = adjustedSpeed;
            }
        });
    }

    int adjustAngle(int angle){
        int adjustedAngle;
        if (angle >= 90 && angle <= 180) { // go left
            adjustedAngle = 90 - angle;
        } else if (angle < 90 && angle >= 0) { // go right
            adjustedAngle = 90 - angle;
        } else if (angle > 0 && angle >= 270){ // go back right
            adjustedAngle = angle - 270;
        } else { // go back left
            adjustedAngle = angle - 270;
        }
        return adjustedAngle;
    }

    int adjustSpeed(int strength, int angle){
        int adjustedSpeed;
        if (angle <= 180) {
            adjustedSpeed = strength;
        } else {
            adjustedSpeed = strength*-1;
        }
        return adjustedSpeed;
    }

}