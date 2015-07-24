package io.sippet.phone;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.app.Activity;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.telephony.PhoneNumberFormattingTextWatcher;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewAnimationUtils;
import android.widget.EditText;
import android.widget.ImageButton;

import com.google.i18n.phonenumbers.NumberParseException;
import com.google.i18n.phonenumbers.PhoneNumberUtil;
import com.google.i18n.phonenumbers.PhoneNumberUtil.PhoneNumberFormat;
import com.google.i18n.phonenumbers.Phonenumber.PhoneNumber;

import java.util.Locale;

public class DialerActivity extends Activity {

    private EditText phoneNumberView;
    private float phoneNumberOriginalSize;
    private ImageButton backspaceButton;
    private PhoneNumberUtil phoneUtil;
    private final int dialpadButtons[] = {
        R.id.zero, R.id.one, R.id.two,
        R.id.three, R.id.four, R.id.five,
        R.id.six, R.id.seven, R.id.eight,
        R.id.nine, R.id.star, R.id.pound
    };
    private FloatingActionButton makeCallButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_dialer);

        phoneUtil = PhoneNumberUtil.getInstance();

        phoneNumberView = (EditText) findViewById(R.id.phone_number);
        phoneNumberOriginalSize = phoneNumberView.getTextSize();
        phoneNumberView.addTextChangedListener(new PhoneNumberFormattingTextWatcher());
        phoneNumberView.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start,
                                          int count, int after) {
            }
            @Override
            public void onTextChanged(CharSequence s, int start,
                                      int before, int count) {
            }
            @Override
            public void afterTextChanged(Editable s) {
                adjustTextSize();
            }
        });

        for (int i = 0; i < dialpadButtons.length; i += 1) {
            final int index = i;
            ImageButton button =
                    (ImageButton) findViewById(dialpadButtons[i]);
            button.setOnClickListener((view) -> {
                int keyCode = KeyEvent.KEYCODE_Z;
                switch (index) {
                    case 0: keyCode = KeyEvent.KEYCODE_0; break;
                    case 1: keyCode = KeyEvent.KEYCODE_1; break;
                    case 2: keyCode = KeyEvent.KEYCODE_2; break;
                    case 3: keyCode = KeyEvent.KEYCODE_3; break;
                    case 4: keyCode = KeyEvent.KEYCODE_4; break;
                    case 5: keyCode = KeyEvent.KEYCODE_5; break;
                    case 6: keyCode = KeyEvent.KEYCODE_6; break;
                    case 7: keyCode = KeyEvent.KEYCODE_7; break;
                    case 8: keyCode = KeyEvent.KEYCODE_8; break;
                    case 9: keyCode = KeyEvent.KEYCODE_9; break;
                    case 10: keyCode = KeyEvent.KEYCODE_STAR; break;
                    case 11: keyCode = KeyEvent.KEYCODE_POUND; break;
                }
                KeyEvent event = new KeyEvent(KeyEvent.ACTION_DOWN, keyCode);
                phoneNumberView.dispatchKeyEvent(event);
            });
        }

        ImageButton zeroButton =
                (ImageButton) findViewById(R.id.zero);
        zeroButton.setOnLongClickListener((view) -> {
            KeyEvent event = new KeyEvent(KeyEvent.ACTION_DOWN,
                    KeyEvent.KEYCODE_PLUS);
            phoneNumberView.dispatchKeyEvent(event);
            return true;
        });

        backspaceButton =
                (ImageButton) findViewById(R.id.backspace);
        backspaceButton.setOnClickListener((view) -> {
            KeyEvent event = new KeyEvent(KeyEvent.ACTION_DOWN,
                    KeyEvent.KEYCODE_DEL);
            phoneNumberView.dispatchKeyEvent(event);
        });
        backspaceButton.setOnLongClickListener((view) -> {
            phoneNumberView.getText().clear();
            return true;
        });

        makeCallButton =
                (FloatingActionButton) findViewById(R.id.make_call);
        makeCallButton.setOnClickListener((view) -> attemptMakeCall());
        makeCallButton.postDelayed(() -> enterReveal(), 0);
    }

    private void adjustTextSize() {
        final float phoneNumberWidth = phoneNumberView.getPaint().measureText(
                phoneNumberView.getText().toString());
        final float backspaceWidth = backspaceButton.getWidth();
        final float frameWidth = phoneNumberView.getWidth();
        final float scale = Resources.getSystem().getDisplayMetrics().density;
        if (phoneNumberWidth >= frameWidth - (backspaceWidth * 2 + 4 * scale)) {
            if (phoneNumberView.getTextSize() > phoneNumberOriginalSize * 0.6) {
                phoneNumberView.setTextSize(
                        phoneNumberView.getTextSize() * 0.95f / scale);
            }
        } else if (phoneNumberWidth < frameWidth - (backspaceWidth * 2 + 8 * scale)) {
            if (phoneNumberView.getTextSize() < phoneNumberOriginalSize) {
                phoneNumberView.setTextSize(
                        phoneNumberView.getTextSize() * 1.05f / scale);
            }
        }
    }

    void enterReveal() {
        // previously invisible view
        final View makeCallView = findViewById(R.id.make_call);

        // get the center for the clipping circle
        int cx = makeCallView.getMeasuredWidth() / 2;
        int cy = makeCallView.getMeasuredHeight() / 2;

        // get the final radius for the clipping circle
        int finalRadius = Math.max(makeCallView.getWidth(), makeCallView.getHeight());

        // create the animator for this view (the start radius is zero)
        Animator anim =
                ViewAnimationUtils.createCircularReveal(makeCallView, cx, cy, 0, finalRadius);
        anim.setDuration(400);

        // make the view visible and start the animation
        makeCallView.setVisibility(View.VISIBLE);
        anim.start();
    }

    void exitReveal() {
        // previously visible view
        final View makeCallView = findViewById(R.id.make_call);

        // get the center for the clipping circle
        int cx = makeCallView.getMeasuredWidth() / 2;
        int cy = makeCallView.getMeasuredHeight() / 2;

        // get the initial radius for the clipping circle
        int initialRadius = makeCallView.getWidth();

        // create the animation (the final radius is zero)
        Animator anim =
                ViewAnimationUtils.createCircularReveal(makeCallView, cx, cy, initialRadius, 0);

        // make the view invisible when the animation is done
        anim.addListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                makeCallView.setVisibility(View.INVISIBLE);
            }
        });
        anim.setDuration(400);

        // start the animation
        anim.start();
    }

    private void attemptMakeCall() {
        // Reset errors.
        phoneNumberView.setError(null);

        // Store values at the time of the makeCall attempt.
        String phoneNumber = phoneNumberView.getText().toString();

        boolean cancel = false;

        // Check for a valid phone number.
        if (TextUtils.isEmpty(phoneNumber)) {
            // Just ignore for now: shall invoke the last dialed number instead
            cancel = true;
        }

        String internationalPhoneNumber = formatNumberToE164(phoneNumber);
        if (internationalPhoneNumber == null) {
            final View snackbar = findViewById(R.id.snackbar);
            Snackbar.make(snackbar, R.string.error_invalid_phone_number, Snackbar.LENGTH_LONG)
                    .setAction("OK", (view) -> { })
                    .setActionTextColor(Color.MAGENTA)
                    .show();
            cancel = true;
        }

        if (!cancel)
            exitReveal();

        // TODO: start the call
    }

    private String formatNumberToE164(String phoneNumber) {
        PhoneNumber proto;
        try {
            proto = phoneUtil.parse(phoneNumber,
                    Locale.getDefault().getCountry());
        } catch (NumberParseException e) {
            return null;
        }
        if (!phoneUtil.isPossibleNumber(proto)) {
            return null;
        }
        return phoneUtil.format(proto, PhoneNumberFormat.E164);
    }
}
