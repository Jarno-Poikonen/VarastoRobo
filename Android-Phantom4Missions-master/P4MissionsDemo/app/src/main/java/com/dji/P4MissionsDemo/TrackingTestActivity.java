

package com.dji.P4MissionsDemo;


import android.annotation.SuppressLint;
import android.graphics.Color;
import android.graphics.RectF;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.TextureView.SurfaceTextureListener;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.SlidingDrawer;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;

import dji.common.camera.SettingsDefinitions;
import dji.common.error.DJIError;
import dji.common.mission.activetrack.ActiveTrackMission;
import dji.common.mission.activetrack.ActiveTrackMissionEvent;
import dji.common.mission.activetrack.ActiveTrackMode;
import dji.common.mission.activetrack.ActiveTrackState;
import dji.common.mission.activetrack.ActiveTrackTargetState;
import dji.common.mission.activetrack.ActiveTrackTrackingState;
import dji.common.mission.activetrack.QuickShotMode;
import dji.common.mission.activetrack.SubjectSensingState;
import dji.common.util.CommonCallbacks;
import dji.common.util.CommonCallbacks.CompletionCallbackWith;
import dji.keysdk.CameraKey;
import dji.keysdk.DJIKey;
import dji.keysdk.FlightControllerKey;
import dji.keysdk.KeyManager;
import dji.keysdk.callback.ActionCallback;
import dji.keysdk.callback.SetCallback;
import dji.log.DJILog;
import dji.midware.media.DJIVideoDataRecver;
import dji.sdk.base.BaseProduct;
import dji.sdk.camera.Camera;
import dji.sdk.media.DownloadListener;
import dji.sdk.media.MediaFile;
import dji.sdk.media.MediaManager;
import dji.sdk.mission.MissionControl;
import dji.sdk.mission.activetrack.ActiveTrackMissionOperatorListener;
import dji.sdk.mission.activetrack.ActiveTrackOperator;
import dji.sdk.products.Aircraft;
import dji.sdk.products.HandHeld;
import dji.sdk.sdkmanager.DJISDKManager;

import static dji.midware.data.manager.P3.ServiceManager.getContext;


public class TrackingTestActivity extends DemoBaseActivity implements SurfaceTextureListener, OnClickListener, OnTouchListener, CompoundButton.OnCheckedChangeListener, ActiveTrackMissionOperatorListener
{
    private static final String TAG = "TrackingTestActivity";
    private static final int MAIN_CAMERA_INDEX = 0;
    private static final int INVALID_INDEX = -1;
    private static final int MOVE_OFFSET = 20;

    private int trackingIndex = INVALID_INDEX;
    private int photoCaptureInterval = 5000;
    private boolean isAutoSensingSupported = false;
    private boolean isDrawingRect = false;

    private RelativeLayout.LayoutParams layoutParams;
    private RelativeLayout mBgLayout;
    private SlidingDrawer mPushInfoSd;
    private TextView mPushInfoTv;
    private ImageView mTrackingImage;
    private ImageView mSendRectIV;
    private ImageButton mStopBtn;
    private Switch mPushBackSw;
    private Switch mQuickShotSw;
    private Switch mAutoCaptureSw;
    private Button mConfirmBtn;
    private Button mRejectBtn;

    private static BaseProduct mProduct;
    private ActiveTrackOperator mActiveTrackOperator;
    private final DJIKey trackModeKey = FlightControllerKey.createFlightAssistantKey(FlightControllerKey.ACTIVE_TRACK_MODE);
    private ConcurrentHashMap<Integer, MultiTrackingView> targetViewHashMap = new ConcurrentHashMap<>();
    private ActiveTrackMode startMode = ActiveTrackMode.TRACE;
    private QuickShotMode quickShotMode = QuickShotMode.UNKNOWN;
    private MediaManager mMediaManager;
    private List<MediaFile> mediaFileList = new ArrayList<>();
    private MediaManager.FileListState currentFileListState = MediaManager.FileListState.UNKNOWN;
    private Timer mTimer1;
    private Handler mTimerHandler = new Handler();
    File destDir = new File(Environment.getExternalStorageDirectory().getPath() + "/TrackerApp/");


    private void startTimer()
    {
        mTimer1 = new Timer();

        TimerTask mTt1 = new TimerTask()
        {
            public void run()
            {
                mTimerHandler.post(() -> capturePhoto());
            }
        };

        mTimer1.schedule(mTt1, photoCaptureInterval, photoCaptureInterval);
    }


    private void stopTimer()
    {
        if ( mTimer1 != null )
        {
            mTimer1.cancel();
            mTimer1.purge();
        }
    }


    private void capturePhoto()
    {
        setResultToToast("Timer!");

        getProductInstance().getCamera().startShootPhoto(djiError ->
        {
            if (null == djiError)
                setResultToToast("Captured successfully");
            else
                setResultToToast(djiError.getDescription());
        });

    }


    // Toast        @param string
    private void setResultToToast(final String msg)
    {
        TrackingTestActivity.this.runOnUiThread(() -> Toast.makeText(TrackingTestActivity.this, msg, Toast.LENGTH_SHORT).show());
    }


    // Push Status to TextView          @param string
    private void setResultToText(final String string)
    {
        if ( mPushInfoTv == null )
            setResultToToast("Push info tv has not be init...");

        TrackingTestActivity.this.runOnUiThread(() -> mPushInfoTv.setText(string));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setContentView(R.layout.activity_tracking_test);
        super.onCreate(savedInstanceState);
        initUI();

        Button buttonDL = findViewById(R.id.buttonDL);

        buttonDL.setOnClickListener(view -> downloadLast());
    }


    // InitUI
    @SuppressLint("ClickableViewAccessibility")
    private void initUI()
    {
        layoutParams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
        ImageButton mPushDrawerIb = findViewById(R.id.tracking_drawer_control_ib);
        mPushInfoSd = findViewById(R.id.tracking_drawer_sd);
        mPushInfoTv = findViewById(R.id.tracking_push_tv);
        mBgLayout = findViewById(R.id.tracking_bg_layout);
        mSendRectIV = findViewById(R.id.tracking_send_rect_iv);
        mTrackingImage = findViewById(R.id.tracking_rst_rect_iv);
        mConfirmBtn = findViewById(R.id.confirm_btn);
        mStopBtn = findViewById(R.id.tracking_stop_btn);
        mRejectBtn = findViewById(R.id.reject_btn);
        mQuickShotSw = findViewById(R.id.set_multiquickshot_enabled);
        mPushBackSw = findViewById(R.id.tracking_pull_back_tb);
        mAutoCaptureSw = findViewById(R.id.switchAutocapture);

        mQuickShotSw.setChecked(false);
        mPushBackSw.setChecked(false);

        mQuickShotSw.setOnCheckedChangeListener(this);
        mPushBackSw.setOnCheckedChangeListener(this);
        mAutoCaptureSw.setOnCheckedChangeListener(this);

        mBgLayout.setOnTouchListener(this);
        mConfirmBtn.setOnClickListener(this);
        mStopBtn.setOnClickListener(this);
        mRejectBtn.setOnClickListener(this);
        mPushDrawerIb.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        initMissionManager();
    }

    /**
     * Init Mission parameter
     */
    private void initMissionManager()
    {
        mActiveTrackOperator = MissionControl.getInstance().getActiveTrackOperator();
        if ( mActiveTrackOperator == null )
            return;

        mActiveTrackOperator.addListener(this);
        mQuickShotSw.setChecked(mActiveTrackOperator.isAutoSensingForQuickShotEnabled());
        mActiveTrackOperator.getRetreatEnabled(new CompletionCallbackWith<Boolean>() {
            @Override
            public void onSuccess(final Boolean aBoolean)
            {
                runOnUiThread(() -> mPushBackSw.setChecked(aBoolean));
            }

            @Override
            public void onFailure(DJIError error)
            {
                setResultToToast("can't get retreat enable state " + error.getDescription());
            }
        });
    }



    // Return BTN response function
    @Override
    public void onReturn(View view)
    {
        DJILog.d(TAG, "onReturn");
        DJISDKManager.getInstance().getMissionControl().destroy();
        this.finish();
    }

    @Override
    protected void onDestroy()
    {
        isAutoSensingSupported = false;
        try {
            DJIVideoDataRecver.getInstance().setVideoDataListener(false, null);
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (mActiveTrackOperator != null)
            mActiveTrackOperator.removeListener(this);

        if (mCodecManager != null)
            mCodecManager.destroyCodec();

        super.onDestroy();
    }

    float downX;
    float downY;


    private double calcManhattanDistance(double point1X, double point1Y, double point2X, double point2Y)
    {
        return Math.abs(point1X - point2X) + Math.abs(point1Y - point2Y);
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouch(View v, MotionEvent event)
    {
        switch (event.getAction())
        {
            case MotionEvent.ACTION_DOWN:
                isDrawingRect = false;
                downX = event.getX();
                downY = event.getY();
                break;

            case MotionEvent.ACTION_MOVE:
                setResultToToast("Currently dragging...");

                if (calcManhattanDistance(downX, downY, event.getX(), event.getY()) < MOVE_OFFSET && !isDrawingRect)
                {
                    trackingIndex = getTrackingIndex(downX, downY, targetViewHashMap);

                    if (targetViewHashMap.get(trackingIndex) != null)
                        Objects.requireNonNull(targetViewHashMap.get(trackingIndex)).setBackgroundColor(Color.RED);

                    setResultToToast("Insufficient movement, no further action");
                    return true;
                }

                isDrawingRect = true;
                mSendRectIV.setVisibility(View.VISIBLE);
                int l = (int) (downX < event.getX() ? downX : event.getX());
                int t = (int) (downY < event.getY() ? downY : event.getY());
                int r = (int) (downX >= event.getX() ? downX : event.getX());
                int b = (int) (downY >= event.getY() ? downY : event.getY());
                mSendRectIV.setX(l);
                mSendRectIV.setY(t);
                mSendRectIV.getLayoutParams().width = r - l;
                mSendRectIV.getLayoutParams().height = b - t;
                mSendRectIV.requestLayout();
                break;

            case MotionEvent.ACTION_UP:
                if (!isDrawingRect)
                {
                    if (targetViewHashMap.get(trackingIndex) != null)
                    {
                        setResultToToast("Selected Index: " + trackingIndex + ",Please Confirm it!");
                        Objects.requireNonNull(targetViewHashMap.get(trackingIndex)).setBackgroundColor(Color.TRANSPARENT);
                    }
                }
                else
                {
                    RectF rectF = getActiveTrackRect(mSendRectIV);
                    ActiveTrackMission mActiveTrackMission = new ActiveTrackMission(rectF, startMode);

                    if (startMode == ActiveTrackMode.QUICK_SHOT)
                    {
                        mActiveTrackMission.setQuickShotMode(quickShotMode);
                        checkStorageStates();
                    }

                    mActiveTrackOperator.startTracking(mActiveTrackMission, error ->
                    {
                        if (error == null)
                            isDrawingRect = false;

                        setResultToToast("Start Tracking: " + (error == null  ?  "Success"  :  error.getDescription()));
                    });
                    mSendRectIV.setVisibility(View.INVISIBLE);
                    clearCurrentView();
                }
                break;

            default:
                break;
        }

        return true;
    }


     // Return
    private int getTrackingIndex(final float x, final float y, final ConcurrentHashMap<Integer, MultiTrackingView> multiTrackinghMap) {
        if (multiTrackinghMap == null || multiTrackinghMap.isEmpty())
            return INVALID_INDEX;

        float l, t, r, b;
        for (Map.Entry<Integer, MultiTrackingView> vo : multiTrackinghMap.entrySet())
        {
            int key = vo.getKey();
            MultiTrackingView view = vo.getValue();
            l = view.getX();
            t = view.getY();
            r = (view.getX() + (view.getWidth() / 2));
            b = (view.getY() + (view.getHeight() / 2));

            if (x >= l && y >= t && x <= r && y <= b)
                return key;
        }
        return INVALID_INDEX;
    }

    @Override
    public void onClick(View v) {
        if (mActiveTrackOperator == null)
            return;

        switch (v.getId())
        {
            case R.id.confirm_btn:
                boolean isAutoTracking =
                        isAutoSensingSupported &&
                                (mActiveTrackOperator.isAutoSensingEnabled() ||
                                        mActiveTrackOperator.isAutoSensingForQuickShotEnabled());
                if (isAutoTracking)
                {
                    startAutoSensingMission();
                    runOnUiThread(() ->
                    {
                        mStopBtn.setVisibility(View.VISIBLE);
                        mRejectBtn.setVisibility(View.VISIBLE);
                        mConfirmBtn.setVisibility(View.INVISIBLE);
                    });
                }
                else
                {
                    trackingIndex = INVALID_INDEX;
                    mActiveTrackOperator.acceptConfirmation(error -> setResultToToast(error == null  ?  "Accept Confirm Success!"  :  error.getDescription()));

                    runOnUiThread(() ->
                    {
                        mStopBtn.setVisibility(View.VISIBLE);
                        mRejectBtn.setVisibility(View.VISIBLE);
                        mConfirmBtn.setVisibility(View.INVISIBLE);
                    });
                }
                break;

            case R.id.tracking_stop_btn:
                trackingIndex = INVALID_INDEX;
                mActiveTrackOperator.stopTracking(error -> setResultToToast(error == null  ?  "Stop track Success!"  :  error.getDescription()));

                runOnUiThread(() ->
                {
                    if ( mTrackingImage != null )
                    {
                        mTrackingImage.setVisibility(View.INVISIBLE);
                        mSendRectIV.setVisibility(View.INVISIBLE);
                        mStopBtn.setVisibility(View.INVISIBLE);
                        mRejectBtn.setVisibility(View.INVISIBLE);
                        mConfirmBtn.setVisibility(View.VISIBLE);
                    }
                });
                break;

            case R.id.reject_btn:
                trackingIndex = INVALID_INDEX;
                mActiveTrackOperator.rejectConfirmation(error -> setResultToToast(error == null  ?  "Reject Confirm Success!"  :  error.getDescription()));
                runOnUiThread(() ->
                {
                    mStopBtn.setVisibility(View.VISIBLE);
                    mRejectBtn.setVisibility(View.VISIBLE);
                    mConfirmBtn.setVisibility(View.INVISIBLE);
                });
                break;

            case R.id.tracking_drawer_control_ib:
                if (mPushInfoSd.isOpened()) {
                    mPushInfoSd.animateClose();
                } else {
                    mPushInfoSd.animateOpen();
                }
                break;

            default:
                break;
        }

    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, final boolean isChecked)
    {
        if (mActiveTrackOperator == null)
            return;

        switch ( compoundButton.getId() )
        {
            case R.id.set_multiquickshot_enabled:
                startMode = ActiveTrackMode.QUICK_SHOT;
                quickShotMode = QuickShotMode.CIRCLE;
                checkStorageStates();
                setAutoSensingForQuickShotEnabled(isChecked);
                break;
            case R.id.tracking_pull_back_tb:
                mActiveTrackOperator.setRetreatEnabled(isChecked, error ->
                {
                    if (error != null)
                        runOnUiThread(() -> mPushBackSw.setChecked(!isChecked));

                    setResultToToast("Set Retreat Enabled: " + (error == null  ?  "Success"  :  error.getDescription()));
                });
                break;
            case R.id.switchAutocapture:
                if (isChecked)
                    startTimer();
                else
                    stopTimer();
                break;
            default:
                break;
        }
    }

    @Override
    public void onUpdate(ActiveTrackMissionEvent event) {
        StringBuffer sb = new StringBuffer();
        String errorInformation = (event.getError() == null ? "null" : event.getError().getDescription()) + "\n";
        String currentState = event.getCurrentState() == null ? "null" : event.getCurrentState().getName();
        String previousState = event.getPreviousState() == null ? "null" : event.getPreviousState().getName();

        ActiveTrackTargetState targetState = ActiveTrackTargetState.UNKNOWN;
        if (event.getTrackingState() != null) {
            targetState = event.getTrackingState().getState();
        }
        Utils.addLineToSB(sb, "CurrentState: ", currentState);
        Utils.addLineToSB(sb, "PreviousState: ", previousState);
        Utils.addLineToSB(sb, "TargetState: ", targetState);
        Utils.addLineToSB(sb, "Error:", errorInformation);

        Object value = KeyManager.getInstance().getValue(trackModeKey);
        if (value instanceof ActiveTrackMode) {
            Utils.addLineToSB(sb, "TrackingMode:", value.toString());
        }

        ActiveTrackTrackingState trackingState = event.getTrackingState();
        if (trackingState != null) {
            final SubjectSensingState[] targetSensingInformations = trackingState.getAutoSensedSubjects();
            if (targetSensingInformations != null) {
                for (SubjectSensingState subjectSensingState : targetSensingInformations) {
                    RectF trackingRect = subjectSensingState.getTargetRect();
                    if (trackingRect != null) {
                        Utils.addLineToSB(sb, "Rect center x: ", trackingRect.centerX());
                        Utils.addLineToSB(sb, "Rect center y: ", trackingRect.centerY());
                        Utils.addLineToSB(sb, "Rect Width: ", trackingRect.width());
                        Utils.addLineToSB(sb, "Rect Height: ", trackingRect.height());
                        Utils.addLineToSB(sb, "Reason", trackingState.getReason().name());
                        Utils.addLineToSB(sb, "Target Index: ", subjectSensingState.getIndex());
                        Utils.addLineToSB(sb, "Target Type", subjectSensingState.getTargetType().name());
                        Utils.addLineToSB(sb, "Target State", subjectSensingState.getState().name());
                        isAutoSensingSupported = true;
                    }
                }
            } else {
                RectF trackingRect = trackingState.getTargetRect();
                if (trackingRect != null) {
                    Utils.addLineToSB(sb, "Rect center x: ", trackingRect.centerX());
                    Utils.addLineToSB(sb, "Rect center y: ", trackingRect.centerY());
                    Utils.addLineToSB(sb, "Rect Width: ", trackingRect.width());
                    Utils.addLineToSB(sb, "Rect Height: ", trackingRect.height());
                    Utils.addLineToSB(sb, "Reason", trackingState.getReason().name());
                    Utils.addLineToSB(sb, "Target Index: ", trackingState.getTargetIndex());
                    Utils.addLineToSB(sb, "Target Type", trackingState.getType().name());
                    Utils.addLineToSB(sb, "Target State", trackingState.getState().name());
                    isAutoSensingSupported = false;
                }
                clearCurrentView();
            }
        }

        setResultToText(sb.toString());
        updateActiveTrackRect(mTrackingImage, event);
        updateButtonVisibility(event);
    }


     // Update ActiveTrack Rect'        @param iv       @param event
    private void updateActiveTrackRect(final ImageView iv, final ActiveTrackMissionEvent event)
    {
        if ( iv == null  ||  event == null )
            return;

        ActiveTrackTrackingState trackingState = event.getTrackingState();

        if (trackingState != null)
        {
            if (trackingState.getAutoSensedSubjects() != null)
            {
                final SubjectSensingState[] targetSensingInformations = trackingState.getAutoSensedSubjects();
                runOnUiThread(() -> updateMultiTrackingView(targetSensingInformations));
            }
            else
            {
                RectF trackingRect = trackingState.getTargetRect();
                ActiveTrackTargetState trackTargetState = trackingState.getState();
                postResultRect(iv, trackingRect, trackTargetState);
            }
        }
    }


    private void updateButtonVisibility(final ActiveTrackMissionEvent event)
    {
        ActiveTrackState state = event.getCurrentState();
        if (state == ActiveTrackState.AUTO_SENSING ||
                state == ActiveTrackState.AUTO_SENSING_FOR_QUICK_SHOT ||
                state == ActiveTrackState.WAITING_FOR_CONFIRMATION)
        {
            TrackingTestActivity.this.runOnUiThread(() ->
            {
                mStopBtn.setVisibility(View.VISIBLE);
                mStopBtn.setClickable(true);
                mConfirmBtn.setVisibility(View.VISIBLE);
                mConfirmBtn.setClickable(true);
                mRejectBtn.setVisibility(View.VISIBLE);
                mRejectBtn.setClickable(true);
            });
        }
        else if (state == ActiveTrackState.AIRCRAFT_FOLLOWING ||
                state == ActiveTrackState.ONLY_CAMERA_FOLLOWING ||
                state == ActiveTrackState.FINDING_TRACKED_TARGET ||
                state == ActiveTrackState.CANNOT_CONFIRM ||
                state == ActiveTrackState.PERFORMING_QUICK_SHOT)
        {
            TrackingTestActivity.this.runOnUiThread(() ->
            {
                mStopBtn.setVisibility(View.VISIBLE);
                mStopBtn.setClickable(true);
                mConfirmBtn.setVisibility(View.INVISIBLE);
                mConfirmBtn.setClickable(false);
                mRejectBtn.setVisibility(View.VISIBLE);
                mRejectBtn.setClickable(true);
            });
        }
        else
        {
            TrackingTestActivity.this.runOnUiThread(() ->
            {
                mStopBtn.setVisibility(View.INVISIBLE);
                mStopBtn.setClickable(false);
                mConfirmBtn.setVisibility(View.INVISIBLE);
                mConfirmBtn.setClickable(false);
                mRejectBtn.setVisibility(View.INVISIBLE);
                mRejectBtn.setClickable(false);
                mTrackingImage.setVisibility(View.INVISIBLE);
            });
        }
    }


    // Get ActiveTrack RectF       @param iv       @return
    private RectF getActiveTrackRect(View iv) {
        View parent = (View) iv.getParent();
        return new RectF(
                ((float) iv.getLeft() + iv.getX()) / (float) parent.getWidth(),
                ((float) iv.getTop() + iv.getY()) / (float) parent.getHeight(),
                ((float) iv.getRight() + iv.getX()) / (float) parent.getWidth(),
                ((float) iv.getBottom() + iv.getY()) / (float) parent.getHeight());
    }


    // Post Result RectF       @param iv       @param rectF        @param targetState
    private void postResultRect(final ImageView iv, final RectF rectF, final ActiveTrackTargetState targetState) {
        View parent = (View) iv.getParent();
        RectF trackingRect = rectF;

        final int l = (int) ((trackingRect.centerX() - trackingRect.width() / 2) * parent.getWidth());
        final int t = (int) ((trackingRect.centerY() - trackingRect.height() / 2) * parent.getHeight());
        final int r = (int) ((trackingRect.centerX() + trackingRect.width() / 2) * parent.getWidth());
        final int b = (int) ((trackingRect.centerY() + trackingRect.height() / 2) * parent.getHeight());

        TrackingTestActivity.this.runOnUiThread(() ->
        {
            mTrackingImage.setVisibility(View.VISIBLE);
            if ( (targetState == ActiveTrackTargetState.CANNOT_CONFIRM)  ||  (targetState == ActiveTrackTargetState.UNKNOWN) )
                iv.setImageResource(R.drawable.visual_track_cannotconfirm);
            else if ( targetState == ActiveTrackTargetState.WAITING_FOR_CONFIRMATION )
                iv.setImageResource(R.drawable.visual_track_needconfirm);
            else if ( targetState == ActiveTrackTargetState.TRACKING_WITH_LOW_CONFIDENCE )
                iv.setImageResource(R.drawable.visual_track_lowconfidence);
            else if ( targetState == ActiveTrackTargetState.TRACKING_WITH_HIGH_CONFIDENCE )
                iv.setImageResource(R.drawable.visual_track_highconfidence);

            iv.setX(l);
            iv.setY(t);
            iv.getLayoutParams().width = r - l;
            iv.getLayoutParams().height = b - t;
            iv.requestLayout();
        });
    }


    // PostMultiResult      @param iv       @param rectF        @param information
    private void postMultiResultRect(final MultiTrackingView iv, final RectF rectF, final SubjectSensingState information)
    {
        View parent = (View) iv.getParent();
        RectF trackingRect = rectF;

        final int l = (int) ((trackingRect.centerX() - trackingRect.width() / 2) * parent.getWidth());
        final int t = (int) ((trackingRect.centerY() - trackingRect.height() / 2) * parent.getHeight());
        final int r = (int) ((trackingRect.centerX() + trackingRect.width() / 2) * parent.getWidth());
        final int b = (int) ((trackingRect.centerY() + trackingRect.height() / 2) * parent.getHeight());

        TrackingTestActivity.this.runOnUiThread(() ->
        {
            mTrackingImage.setVisibility(View.INVISIBLE);
            iv.setX(l);
            iv.setY(t);
            iv.getLayoutParams().width = r - l;
            iv.getLayoutParams().height = b - t;
            iv.requestLayout();
            iv.updateView(information);
        });
    }


    // Update MultiTrackingView     @param targetSensingInformations
    private void updateMultiTrackingView(final SubjectSensingState[] targetSensingInformations) {
        ArrayList<Integer> indexs = new ArrayList<>();
        for (SubjectSensingState target : targetSensingInformations) {
            indexs.add(target.getIndex());
            if (targetViewHashMap.containsKey(target.getIndex())) {

                MultiTrackingView targetView = targetViewHashMap.get(target.getIndex());
                postMultiResultRect(targetView, target.getTargetRect(), target);
            } else {
                MultiTrackingView trackingView = new MultiTrackingView(TrackingTestActivity.this);
                mBgLayout.addView(trackingView, layoutParams);
                targetViewHashMap.put(target.getIndex(), trackingView);
            }
        }

        ArrayList<Integer> missingIndexs = new ArrayList<>();
        for (Integer key : targetViewHashMap.keySet()) {
            boolean isDisappeared = true;
            for (Integer index : indexs) {
                if (index.equals(key)) {
                    isDisappeared = false;
                    break;
                }
            }

            if (isDisappeared) {
                missingIndexs.add(key);
            }
        }

        for (Integer i : missingIndexs) {
            MultiTrackingView view = targetViewHashMap.remove(i);
            mBgLayout.removeView(view);
        }
    }



     // Enable QuickShotMode        @param isChecked
    private void setAutoSensingForQuickShotEnabled(final boolean isChecked)
    {
        if (mActiveTrackOperator != null)
        {
            if (isChecked)
            {
                mActiveTrackOperator.enableAutoSensingForQuickShot(error ->
                {
                    if ( error != null )
                        runOnUiThread(() -> mQuickShotSw.setChecked(!isChecked));

                    setResultToToast("Set QuickShot Enabled " + (error == null ? "Success!" : error.getDescription()));
                });

            }
            else
                disableAutoSensing();
        }
    }


    // Disable AutoSensing
    private void disableAutoSensing()
    {
        if ( mActiveTrackOperator != null )
        {
            mActiveTrackOperator.disableAutoSensing(error ->
            {
                if ( error == null )
                {
                    runOnUiThread(() ->
                    {
                        mConfirmBtn.setVisibility(View.INVISIBLE);
                        mStopBtn.setVisibility(View.INVISIBLE);
                        mRejectBtn.setVisibility(View.INVISIBLE);
                        isAutoSensingSupported = false;
                    });
                    clearCurrentView();
                }
                setResultToToast(error == null ? "Disable Auto Sensing Success!" : error.getDescription());
            });
        }
    }


    // Confirm Mission by Index
    private void startAutoSensingMission()
    {
        if (trackingIndex != INVALID_INDEX)
        {
            ActiveTrackMission mission = new ActiveTrackMission(null, startMode);
            mission.setQuickShotMode(quickShotMode);
            mission.setTargetIndex(trackingIndex);
            mActiveTrackOperator.startAutoSensingMission(mission, error ->
            {
                if (error == null)
                {
                    setResultToToast("Accept Confirm index: " + trackingIndex + " Success!");
                    trackingIndex = INVALID_INDEX;
                }
                else
                    setResultToToast(error.getDescription());
            });
        }
    }


    // Change Storage Location
    private void switchStorageLocation(final SettingsDefinitions.StorageLocation storageLocation)
    {
        KeyManager keyManager = KeyManager.getInstance();
        DJIKey storageLocationkey = CameraKey.create(CameraKey.CAMERA_STORAGE_LOCATION, MAIN_CAMERA_INDEX);

        if (storageLocation == SettingsDefinitions.StorageLocation.INTERNAL_STORAGE)
        {
            keyManager.setValue(storageLocationkey, SettingsDefinitions.StorageLocation.SDCARD, new SetCallback()
            {
                @Override
                public void onSuccess() {
                    setResultToToast("Change to SD card Success!");
                }

                @Override
                public void onFailure(@NonNull DJIError error) {
                    setResultToToast(error.getDescription());
                }
            });
        }
        else
        {
            keyManager.setValue(storageLocationkey, SettingsDefinitions.StorageLocation.INTERNAL_STORAGE, new SetCallback()
            {
                @Override
                public void onSuccess()
                {
                    setResultToToast("Change to Interal Storage Success!");
                }

                @Override
                public void onFailure(@NonNull DJIError error)
                {
                    setResultToToast(error.getDescription());
                }
            });
        }
    }


    // Determine SD Card is or not Ready        @param index        @return
    private boolean isSDCardReady(int index)
    {
        KeyManager keyManager = KeyManager.getInstance();

        return ((Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_IS_INSERTED, index))
                && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_IS_INITIALIZING, index))
                && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_IS_READ_ONLY, index))
                && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_HAS_ERROR, index))
                && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_IS_FULL, index))
                && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_IS_BUSY, index))
                && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_IS_FORMATTING, index))
                && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_IS_INVALID_FORMAT, index))
                && (Boolean) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_IS_VERIFIED, index))
                && (Long) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_AVAILABLE_CAPTURE_COUNT, index)) > 0L
                && (Integer) keyManager.getValue(CameraKey.create(CameraKey.SDCARD_AVAILABLE_RECORDING_TIME_IN_SECONDS, index)) > 0);
    }


    // Determine Interal Storage is or not Ready        @param index        @return
    private boolean isInternalStorageReady(int index)
    {
        KeyManager keyManager = KeyManager.getInstance();

        boolean isInternalSupported = (boolean)
                keyManager.getValue(CameraKey.create(CameraKey.IS_INTERNAL_STORAGE_SUPPORTED, index));
        if (isInternalSupported) {
            return ((Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_IS_INSERTED, index))
                    && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_IS_INITIALIZING, index))
                    && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_IS_READ_ONLY, index))
                    && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_HAS_ERROR, index))
                    && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_IS_FULL, index))
                    && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_IS_BUSY, index))
                    && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_IS_FORMATTING, index))
                    && !(Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_IS_INVALID_FORMAT, index))
                    && (Boolean) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_IS_VERIFIED, index))
                    && (Long) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_AVAILABLE_CAPTURE_COUNT, index)) > 0L
                    && (Integer) keyManager.getValue(CameraKey.create(CameraKey.INNERSTORAGE_AVAILABLE_RECORDING_TIME_IN_SECONDS, index)) > 0);
        }
        return false;
    }


    // Check Storage States
    private void checkStorageStates()
    {
        KeyManager keyManager = KeyManager.getInstance();
        DJIKey storageLocationkey = CameraKey.create(CameraKey.CAMERA_STORAGE_LOCATION, MAIN_CAMERA_INDEX);
        Object storageLocationObj = keyManager.getValue(storageLocationkey);
        SettingsDefinitions.StorageLocation storageLocation = SettingsDefinitions.StorageLocation.INTERNAL_STORAGE;

        if (storageLocationObj instanceof SettingsDefinitions.StorageLocation)
            storageLocation = (SettingsDefinitions.StorageLocation) storageLocationObj;

        if (storageLocation == SettingsDefinitions.StorageLocation.INTERNAL_STORAGE)
        {
            if (!isInternalStorageReady(MAIN_CAMERA_INDEX) && isSDCardReady(MAIN_CAMERA_INDEX))
                switchStorageLocation(SettingsDefinitions.StorageLocation.SDCARD);
        }

        if (storageLocation == SettingsDefinitions.StorageLocation.SDCARD)
        {
            if (!isSDCardReady(MAIN_CAMERA_INDEX) && isInternalStorageReady(MAIN_CAMERA_INDEX))
                switchStorageLocation(SettingsDefinitions.StorageLocation.INTERNAL_STORAGE);
        }

        DJIKey isRecordingKey = CameraKey.create(CameraKey.IS_RECORDING, MAIN_CAMERA_INDEX);
        Object isRecording = keyManager.getValue(isRecordingKey);
        if ( isRecording instanceof Boolean )
        {
            if ( (Boolean) isRecording )
            {
                keyManager.performAction(CameraKey.create(CameraKey.STOP_RECORD_VIDEO, MAIN_CAMERA_INDEX), new ActionCallback() {
                    @Override
                    public void onSuccess() {
                        setResultToToast("Stop Recording Success!");
                    }

                    @Override
                    public void onFailure(@NonNull DJIError error) {
                        setResultToToast("Stop Recording Fail，Error " + error.getDescription());
                    }
                });
            }
        }
    }


    // Clear MultiTracking View
    private void clearCurrentView()
    {
        if ( targetViewHashMap != null && !targetViewHashMap.isEmpty() )
        {
            Iterator<Map.Entry<Integer, MultiTrackingView>> it = targetViewHashMap.entrySet().iterator();

            while ( it.hasNext() )
            {
                Map.Entry<Integer, MultiTrackingView> entry = it.next();
                final MultiTrackingView view = entry.getValue();
                it.remove();
                TrackingTestActivity.this.runOnUiThread(() -> mBgLayout.removeView(view));
            }
        }
    }





    private void downloadLast()     // Download last image
    {
        setResultToToast("Download button pressed.");

        getFileList();

        setResultToToast( String.valueOf(mediaFileList.size()) );

        if ( mediaFileList.size() == 0 )
            return;

        mediaFileList.get(mediaFileList.size() - 1).fetchFileData(destDir, null, new DownloadListener<String>()
        {
            @Override
            public void onFailure(DJIError error) {
                setResultToToast("Download File Failed" + error.getDescription());
            }

            @Override
            public void onProgress(long total, long current) {
            }

            @Override
            public void onRateUpdate(long total, long current, long persize) {
            }

            @Override
            public void onStart() {
                setResultToToast("Started download.");
            }

            @Override
            public void onSuccess(String filePath) {
                setResultToToast("Download File Success:" + filePath);
            }
        });
    }


    private void getFileList()
    {
        mMediaManager = Objects.requireNonNull(getCameraInstance()).getMediaManager();

        if ( mMediaManager != null )
        {
            if ( (currentFileListState == MediaManager.FileListState.SYNCING) || (currentFileListState == MediaManager.FileListState.DELETING) )
                DJILog.e(TAG, "Media Manager is busy.");
            else
            {
                mMediaManager.refreshFileListOfStorageLocation(SettingsDefinitions.StorageLocation.SDCARD, djiError ->
                {
                    if ( null == djiError )
                    {
                        mediaFileList.clear();
                        mediaFileList = mMediaManager.getSDCardFileListSnapshot();

                        Collections.sort(mediaFileList, (lhs, rhs) ->
                        {
                            if ( lhs.getTimeCreated() < rhs.getTimeCreated() )
                                return 1;
                            else if ( lhs.getTimeCreated() > rhs.getTimeCreated() )
                                return -1;

                            return 0;
                        });
                    }
                    else
                        setResultToToast("Get Media File List Failed:" + djiError.getDescription());
                });
            }
        }
    }


    public static synchronized BaseProduct getProductInstance()
    {
        if ( mProduct == null )
            mProduct = DJISDKManager.getInstance().getProduct();

        return mProduct;
    }


    public static synchronized Camera getCameraInstance()
    {
        return getProductInstance() == null  ?  null  :  (getProductInstance()).getCamera();
    }
}

