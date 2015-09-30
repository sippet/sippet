package io.sippet.demo;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.os.Bundle;
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
import io.sippet.phone.OnIncomingCallListener;
import io.sippet.phone.Phone;
import io.sippet.phone.Settings;

public class SippetApplication extends Application {
    private final String TAG = "SippetApplication";

    private Phone mPhone;
    private SippetActivity mForegroundActivity;

    @Override
    public void onCreate() {
        super.onCreate();

        registerActivityLifecycleCallbacks(new ActivityLifecycleCallbacks() {
            @Override
            public void onActivityCreated(Activity activity, Bundle savedInstanceState) {}

            @Override
            public void onActivityStarted(Activity activity) {}

            @Override
            public void onActivityResumed(Activity activity) {
                mForegroundActivity = (SippetActivity)activity;
            }

            @Override
            public void onActivityPaused(Activity activity) {
                mForegroundActivity = null;
            }

            @Override
            public void onActivityStopped(Activity activity) {}

            @Override
            public void onActivitySaveInstanceState(Activity activity, Bundle outState) {}

            @Override
            public void onActivityDestroyed(Activity activity) {}
        });

        Phone.loadLibraries(getApplicationContext());
        mPhone = new Phone(new OnIncomingCallListener() {
                    @Override
                    public void onIncomingCall(Call call) {
                        if (mForegroundActivity != null)
                            mForegroundActivity.onIncomingCall(call);
                    }
                });
    }

    public Phone getPhone() {
        return mPhone;
    }
}
