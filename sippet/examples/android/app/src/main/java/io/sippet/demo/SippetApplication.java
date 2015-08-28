package io.sippet.demo;

import android.app.Application;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.support.annotation.NonNull;
import android.util.Log;

import org.chromium.base.ResourceExtractor;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

import io.sippet.phone.Call;
import io.sippet.phone.Phone;
import io.sippet.phone.Settings;

public class SippetApplication extends Application {
    private final String TAG = "SippetApplication";

    private Phone mPhone;

    @Override
    public void onCreate() {
        super.onCreate();

        mPhone = new Phone(getApplicationContext(),
                new Phone.Delegate() {
                    @Override
                    public void onNetworkError(int error) {

                    }

                    @Override
                    public void onRegisterCompleted(int status_code,
                                                    String status_text) {
                        Log.d(TAG, String.format("onRegisterCompleted(%d, '%s')", status_code, status_text));
                    }

                    @Override
                    public void onRefreshError(int status_code,
                                               String status_text) {

                    }

                    @Override
                    public void onUnregisterCompleted(int status_code,
                                                      String status_text) {

                    }

                    @Override
                    public void onIncomingCall(Call call) {

                    }
                });

        Settings settings = new Settings();
        settings.setDisableEncryption(true);
        settings.setDisableSctpDataChannels(true);
        settings.setUri("sip:helloworld@foo.com");
        settings.setPassword("s3cr3t");
        List<String> routeSet = Arrays.asList("sip:bar.com;lr");
        settings.setRouteSet(routeSet);
        if (mPhone.init(settings)) {
            Log.d(TAG, "Initialized");
            mPhone.register();
        }
        int state = mPhone.getState();
    }
}
