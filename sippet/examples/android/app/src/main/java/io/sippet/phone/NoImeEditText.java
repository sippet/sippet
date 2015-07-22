package io.sippet.phone;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.EditText;

/**
 * An EditText field that inhibits the soft keyboard.
 */
public class NoImeEditText extends EditText {
    public NoImeEditText(Context context) {
        super(context);
    }

    public NoImeEditText(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public NoImeEditText(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public boolean onCheckIsTextEditor() {
        return false;
    }
}
