package io.sippet.phone;

import android.content.Context;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;

/**
 * An EditText field that inhibits the soft keyboard.
 */
public class NoImeEditText extends EditText {
    public NoImeEditText(Context context) {
        super(context);
        hideKeyboard();
    }

    public NoImeEditText(Context context, AttributeSet attrs) {
        super(context, attrs);
        hideKeyboard();
    }

    public NoImeEditText(Context context, AttributeSet attrs,
                         int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        hideKeyboard();
    }

    @Override
    protected void onFocusChanged(boolean focused, int direction,
                                  Rect previouslyFocusedRect) {
        hideKeyboard();
        super.onFocusChanged(focused, direction, previouslyFocusedRect);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        boolean ret = super.onTouchEvent(event);
        // Must be done after super.onTouchEvent()
        hideKeyboard();
        return ret;
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right,
                            int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        // Here we ensure that we hide the keyboard.
        // This possibly blinks the keyboard but for now there's no better way
        hideKeyboard();
    }

    @Override
    public void sendAccessibilityEventUnchecked(AccessibilityEvent event) {
        if (event.getEventType() == AccessibilityEvent.TYPE_VIEW_FOCUSED) {
            // The parent EditText class lets tts read "edit box" when this View
            // has a focus, which
            // confuses users on app launch (issue 5275935).
            return;
        }
        super.sendAccessibilityEventUnchecked(event);
    }

    private void hideKeyboard() {
        final InputMethodManager imm = ((InputMethodManager) getContext()
                .getSystemService(Context.INPUT_METHOD_SERVICE));
        if (imm != null) {
            if (imm.isActive(this)) {
                imm.hideSoftInputFromWindow(getApplicationWindowToken(), 0);
            }
        }
    }
}
