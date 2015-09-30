package io.sippet.demo;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.LoaderManager.LoaderCallbacks;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.Loader;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;

import android.os.Build;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import io.sippet.phone.Call;
import io.sippet.phone.CompletionCallback;
import io.sippet.phone.Phone;
import io.sippet.phone.Settings;


/**
 * A login screen that offers login via uri/password/registrar.
 */
public class LoginActivity extends SippetActivity {
    private final String TAG = "LoginActivity";

    /**
     * A dummy authentication store containing known user names and passwords.
     * TODO: remove after connecting to a real Sippet Phone instance.
     */
    private static final String[] DUMMY_CREDENTIALS = new String[]{
            "sip:foo@example.com:hello", "sip:bar@example.com:world"
    };

    /**
     * A dummy registrar store containing known registrar servers.
     * TODO: remove after connecting to the real Sippet Phone instance.
     */
    private static final String[] DUMMY_REGISTRARS = new String[]{
            "sip:foo@example.com:hello", "sip:bar@example.com:world"
    };

    // UI references.
    private EditText uriView;
    private EditText passwordView;
    private EditText registrarServerView;
    private EditText routeView;
    private View progressView;
    private View loginFormView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        // Set up the login form.
        uriView = (EditText) findViewById(R.id.uri);
        passwordView = (EditText) findViewById(R.id.password);
        registrarServerView = (EditText) findViewById(R.id.registrar_server);
        routeView = (EditText) findViewById(R.id.route);

        ImageButton signInButton = (ImageButton) findViewById(R.id.sign_in_button);
        signInButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                attemptLogin();
            }
        });

        loginFormView = findViewById(R.id.login_form);
        progressView = findViewById(R.id.login_progress);

        restoreForm();
    }

    /**
     * Attempts to sign in or register the account specified by the login form.
     * If there are form errors (invalid email, missing fields, etc.), the
     * errors are presented and no actual login attempt is made.
     */
    public void attemptLogin() {

        saveForm();

        // Reset errors.
        uriView.setError(null);
        passwordView.setError(null);
        registrarServerView.setError(null);

        // Store values at the time of the login attempt.
        String uri = uriView.getText().toString();
        String password = passwordView.getText().toString();
        String registrarServer = registrarServerView.getText().toString();
        String route = routeView.getText().toString();

        boolean cancel = false;
        View focusView = null;

        // Check for a valid password, if the user entered one.
        if (!TextUtils.isEmpty(password) && !isPasswordValid(password)) {
            passwordView.setError(getString(R.string.error_invalid_password));
            focusView = passwordView;
            cancel = true;
        }

        // Check for a valid email address.
        if (TextUtils.isEmpty(uri)) {
            uriView.setError(getString(R.string.error_field_required));
            focusView = uriView;
            cancel = true;
        } else if (!isUriValid(uri)) {
            uriView.setError(getString(R.string.error_invalid_uri));
            focusView = uriView;
            cancel = true;
        }

        // Check for a valid registrar server.
        if (!TextUtils.isEmpty(registrarServer)) {
            if (!isRegistrarValid(registrarServer)) {
                registrarServerView.setError(getString(R.string.error_invalid_uri));
                focusView = registrarServerView;
                cancel = true;
            }
        }

        if (!cancel) {
            Settings settings = new Settings();
            settings.setDisableEncryption(true);
            settings.setDisableSctpDataChannels(true);
            settings.setUri(uri);
            settings.setPassword(password);
            if (!TextUtils.isEmpty(registrarServer)) {
                settings.setRegistrarServer(registrarServer);
            }
            if (!TextUtils.isEmpty(route)) {
                List<String> routeSet = Arrays.asList(route.split(" *, *"));
                settings.setRouteSet(routeSet);
            }
            if (!getPhone().init(settings)) {
                // TODO: show the error in an appropriated way
                Log.d(TAG, "Error in provided settings");
                focusView = registrarServerView;
                cancel = true;
            }
        }

        if (cancel) {
            // There was an error; don't attempt login and focus the first
            // form field with an error.
            focusView.requestFocus();
        } else {
            // Show a progress spinner, and kick off a background task to
            // perform the user login attempt.
            showProgress(true);

            Log.d(TAG, "Registering...");
            getPhone().register(new CompletionCallback() {
                @Override
                public void onCompleted(int error) {
                    showProgress(false);
                    if (error == 0) {
                        loadDialer();
                    } else {
                        // TODO: handle other errors
                        passwordView.setError(getString(R.string.error_incorrect_password));
                        passwordView.requestFocus();
                    }
                }
            });
        }
    }

    public void saveForm() {
        SharedPreferences preferences = getPreferences(MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString("settings_uri", uriView.getText().toString());
        editor.putString("settings_password", passwordView.getText().toString());
        editor.putString("settings_registrar_server", registrarServerView.getText().toString());
        editor.putString("settings_route_set", routeView.getText().toString());
        editor.commit();
    }

    public void restoreForm() {
        SharedPreferences preferences = getPreferences(MODE_PRIVATE);
        uriView.setText(preferences.getString("settings_uri", ""));
        passwordView.setText(preferences.getString("settings_password", ""));
        registrarServerView.setText(preferences.getString("settings_registrar_server", ""));
        routeView.setText(preferences.getString("settings_route_set", ""));
    }

    private boolean isUriValid(String uri) {
        return uri.contains("@") &&
                (uri.startsWith("sip:") || uri.startsWith("sips:"));
    }

    private boolean isRegistrarValid(String uri) {
        return !uri.contains("@") &&
                (uri.startsWith("sip:") || uri.startsWith("sips:"));
    }

    private boolean isPasswordValid(String password) {
        return password.length() >= 4;
    }

    /**
     * Shows the progress UI and hides the login form.
     */
    @TargetApi(Build.VERSION_CODES.HONEYCOMB_MR2)
    public void showProgress(final boolean show) {
        // On Honeycomb MR2 we have the ViewPropertyAnimator APIs, which allow
        // for very easy animations. If available, use these APIs to fade-in
        // the progress spinner.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR2) {
            int shortAnimTime = getResources().getInteger(android.R.integer.config_shortAnimTime);

            loginFormView.setVisibility(show ? View.GONE : View.VISIBLE);
            loginFormView.animate().setDuration(shortAnimTime).alpha(
                    show ? 0 : 1).setListener(new AnimatorListenerAdapter() {
                @Override
                public void onAnimationEnd(Animator animation) {
                    loginFormView.setVisibility(show ? View.GONE : View.VISIBLE);
                }
            });

            progressView.setVisibility(show ? View.VISIBLE : View.GONE);
            progressView.animate().setDuration(shortAnimTime).alpha(
                    show ? 1 : 0).setListener(new AnimatorListenerAdapter() {
                @Override
                public void onAnimationEnd(Animator animation) {
                    progressView.setVisibility(show ? View.VISIBLE : View.GONE);
                }
            });
        } else {
            // The ViewPropertyAnimator APIs are not available, so simply show
            // and hide the relevant UI components.
            progressView.setVisibility(show ? View.VISIBLE : View.GONE);
            loginFormView.setVisibility(show ? View.GONE : View.VISIBLE);
        }
    }

    @Override
    protected void onIncomingCall(Call call) {
        // TODO
    }

    private void loadDialer() {
        Intent intent = new Intent(this, DialerActivity.class);
        startActivity(intent);
    }
}

