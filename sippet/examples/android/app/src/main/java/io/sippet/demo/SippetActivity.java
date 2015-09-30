package io.sippet.demo;

import android.app.Activity;

import io.sippet.phone.Call;
import io.sippet.phone.Phone;

public abstract class SippetActivity extends Activity {
    protected abstract void onIncomingCall(Call call);

    protected Phone getPhone() {
        SippetApplication application = (SippetApplication)getApplication();
        return application.getPhone();
    }
}
