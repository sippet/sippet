package io.sippet.phone;

import android.app.Activity;
import android.os.Bundle;
import android.telephony.PhoneNumberFormattingTextWatcher;
import android.widget.EditText;
import android.widget.ImageButton;

public class DialerActivity extends Activity {

    private EditText phoneNumberView;
    private final int dialpadButtons[] = {
        R.id.zero, R.id.one, R.id.two,
        R.id.three, R.id.four, R.id.five,
        R.id.six, R.id.seven, R.id.eight,
        R.id.nine, R.id.star, R.id.pound
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_dialer);

        phoneNumberView = (EditText) findViewById(R.id.phone_number);
        phoneNumberView.addTextChangedListener(new PhoneNumberFormattingTextWatcher());
        for (int i = 0; i < dialpadButtons.length; i += 1) {
            final int index = i;
            ImageButton button =
                (ImageButton) findViewById(dialpadButtons[i]);
            button.setOnClickListener(
                (view) -> {
                    if (index < 10) {
                        phoneNumberView.append(Integer.toString(index));
                    } else if (index == 10) {
                        phoneNumberView.append("*");
                    } else if (index == 11) {
                        phoneNumberView.append("#");
                    }
                    phoneNumberView.setSelection(
                        phoneNumberView.getText().length());
                });
        }

        ImageButton zeroButton =
            (ImageButton) findViewById(R.id.zero);
        zeroButton.setOnLongClickListener(
                (view) -> {
                    phoneNumberView.append("+");
                    phoneNumberView.setSelection(
                        phoneNumberView.getText().length());
                    return true;
                });

        ImageButton backspaceButton = (ImageButton) findViewById(R.id.backspace);
        backspaceButton.setOnClickListener((view) -> {
            int length = phoneNumberView.getText().length();
            if (length > 0) {
                phoneNumberView.getText().delete(length - 1, length);
                phoneNumberView.setSelection(
                    phoneNumberView.getText().length());
            }
        });
    }
}
