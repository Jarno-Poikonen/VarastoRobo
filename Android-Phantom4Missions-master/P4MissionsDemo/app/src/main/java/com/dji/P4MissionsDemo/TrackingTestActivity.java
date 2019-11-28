

package com.dji.P4MissionsDemo;


import android.graphics.Color;
import android.graphics.RectF;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.view.TextureView.SurfaceTextureListener;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.SlidingDrawer;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
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
import dji.common.mission.activetrack.ActiveTrackTargetType;
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
import dji.sdk.sdkmanager.DJISDKManager;


public class TrackingTestActivity extends DemoBaseActivity implements SurfaceTextureListener, OnClickListener, CompoundButton.OnCheckedChangeListener, ActiveTrackMissionOperatorListener
{
    private static final String TAG = "TrackingTestActivity";
    private static final int MAIN_CAMERA_INDEX = 0;
    private static final int INVALID_INDEX = -1;

    private int trackingIndex = INVALID_INDEX;
    private int photoCaptureInterval = 5000;
    private int stopAlertInterval = 500;
    private int stopAlertCounter = 0;
    private int SERVER_PORT = 5000;

    private boolean isAutoSensingSupported = false;
    private boolean stopAlertVisible = false;

    private RelativeLayout.LayoutParams layoutParams;
    private RelativeLayout mBgLayout;
    private SlidingDrawer mPushInfoSd;
    private TextView mPushInfoTv;
    private TextView mStopMessage;
    private TextView mTextIP;
    private TextView mClientStatus;
    private ImageView mTrackingImage;
    private ImageButton mStopBtn;
    private ImageButton mPushDrawerIb;
    private Switch mPushBackSw;
    private Switch mQuickShotSw;
    private Switch mAutoCaptureSw;
    private Button mConfirmBtn;
    private Button mRejectBtn;
    private Handler mTimerHandler = new Handler();
    private ActiveTrackMission mActiveTrackMission;
    private static BaseProduct mProduct;
    private ActiveTrackOperator mActiveTrackOperator;
    private final DJIKey trackModeKey = FlightControllerKey.createFlightAssistantKey(FlightControllerKey.ACTIVE_TRACK_MODE);
    private ConcurrentHashMap<Integer, MultiTrackingView> targetViewHashMap = new ConcurrentHashMap<>();
    private ActiveTrackMode startMode = ActiveTrackMode.TRACE;
    private QuickShotMode quickShotMode = QuickShotMode.UNKNOWN;
    private MediaManager mMediaManager;
    private List<MediaFile> mediaFileList = new ArrayList<>();
    private MediaManager.FileListState currentFileListState = MediaManager.FileListState.UNKNOWN;
    private Timer photoCaptureTimer;
    private Timer stopAlertTimer;
    private Handler photoCaptureTimerHandler = new Handler();
    private Handler stopAlertTimerHandler = new Handler();
    File destDir = new File(Environment.getExternalStorageDirectory().getPath() + "/TrackerApp/");
    private Timer mTimer1;
    private boolean flag = true;
    ServerSocket serverSocket;
    Socket clientSocket;
    Thread SocketThread = null;
    BufferedReader input;
    float i = 0.0f;
    float ret = 0.0f;


    // Toast        @param string
    private void writeToast(final String msg)
    {
        TrackingTestActivity.this.runOnUiThread(() -> Toast.makeText(TrackingTestActivity.this, msg, Toast.LENGTH_SHORT).show());
    }


    // Push Status to TextView          @param string
    private void setResultToText(final String string)
    {
        if ( mPushInfoTv == null )
            writeToast("Push info tv has not be init...");

        TrackingTestActivity.this.runOnUiThread(() -> mPushInfoTv.setText(string));
    }

    private void setResultToToast(final String string) {
        TrackingTestActivity.this.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(TrackingTestActivity.this, string, Toast.LENGTH_SHORT).show();
            }
        });
    }
    // @Override
    private void startTimer()
    {
        mTimer1 = new Timer();

        TimerTask mTt1 = new TimerTask()
        {
            public void run()
            {
                mTimerHandler.post(() -> track());

            }
        };

        mTimer1.schedule(mTt1, 2000, 2000);
    }


    private void stopTimer()
    {
        if ( mTimer1 != null )
        {
            mTimer1.cancel();
            mTimer1.purge();
        }
    }

    private float moveX(){
        if(i<0.8f) {
            i = i + 0.05f;
        }
        else{
            i = 0.0f;
        }
        setResultToToast("i:"+i);
        return i;
    }
    TextView textScreen;

    public void track()
    {
        /*
        WiFiLink wifilink = new WiFiLink();
        wifilink.getChannelNumber(new CompletionCallbackWith<Integer>() {
            @Override
            public void onSuccess(Integer channelNumber) {
                //runOnUiThread(()->textScreen.setText("channelNumber"+channelNumber));
            }

            @Override
            public void onFailure(DJIError djiError) {
                //runOnUiThread(()->textScreen.setText("Failed to get channelNumber"+djiError));
            }
        });
        */

      /*
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                isDrawingRect = false;
                downX = event.getX();
                downY = event.getY();
                break;

            case MotionEvent.ACTION_MOVE:
                if (calcManhattanDistance(downX, downY, event.getX(), event.getY()) < MOVE_OFFSET && !isDrawingRect) {
                trackingIndex = getTrackingIndex(downX, downY, targetViewHashMap);

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
            */

        // float downX = 0.2f;
        // float downY = 0.2f;

        //  if (calcManhattanDistance(downX, downY, 0.5f, 0.5f) < MOVE_OFFSET && !isDrawingRect) {


        //  trackingIndex = getTrackingIndex(downX, downY, targetViewHashMap);
        //  DJILog.d(TAG,trackingIndex);
        trackingIndex = INVALID_INDEX;
        // mSendRectIV.setVisibility(View.VISIBLE);
        //mSendRectIV.setX(0.3f);
        // mSendRectIV.setY(0.7f);
        //mSendRectIV.getLayoutParams().width = (int)0.2;
        // mSendRectIV.getLayoutParams().height = (int)0.6;
        //    mSendRectIV.requestLayout();



        //RectF rectF = getActiveTrackRect(mSendRectIV);
        if (flag) {
            ret = moveX();

            // for (float i = 0.0f; i<0.4f;i=i+0.1f){
            RectF rectF = new RectF(0.1f + ret, 0.2f, 0.2f + ret, 0.8f);

            if (targetViewHashMap.get(trackingIndex) != null) {
                Objects.requireNonNull(targetViewHashMap.get(trackingIndex)).setBackgroundColor(Color.RED);
            }

            mActiveTrackMission = new ActiveTrackMission(rectF, startMode);

            //  }


            if (startMode == ActiveTrackMode.QUICK_SHOT) {
                mActiveTrackMission.setQuickShotMode(quickShotMode);
                checkStorageStates();
            }

            mActiveTrackOperator.startTracking(mActiveTrackMission, new CommonCallbacks.CompletionCallback() {
                @Override
                public void onResult(DJIError error) {
                    if (error == null) {

                    }
                    //stopTimer();
                    setResultToToast("Start Tracking: " + (error == null
                            ? "Success"
                            : error.getDescription()));

                }
            });
            clearCurrentView();
        }
    }
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        setContentView(R.layout.activity_tracking_test);
        super.onCreate(savedInstanceState);
        initUI();
        serverSetup();
    }


    // InitUI
    private void initUI()
    {
        startTimer();
        layoutParams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);

        mPushDrawerIb = findViewById(R.id.tracking_drawer_control_ib);
        mPushInfoSd = findViewById(R.id.tracking_drawer_sd);
        mPushInfoTv = findViewById(R.id.tracking_push_tv);
        mBgLayout = findViewById(R.id.tracking_bg_layout);
        mTrackingImage = findViewById(R.id.tracking_rst_rect_iv);
        mConfirmBtn = findViewById(R.id.confirm_btn);
        mStopBtn = findViewById(R.id.tracking_stop_btn);
        mRejectBtn = findViewById(R.id.reject_btn);
        mQuickShotSw = findViewById(R.id.set_multiquickshot_enabled);
        mPushBackSw = findViewById(R.id.tracking_pull_back_tb);
        mAutoCaptureSw = findViewById(R.id.switchAutocapture);
        mStopMessage = findViewById(R.id.stopMessage);
        mTextIP = findViewById(R.id.textIP);
        mClientStatus = findViewById(R.id.textClientStatus);

        mQuickShotSw.setChecked(false);
        mPushBackSw.setChecked(false);
        mStopMessage.setFocusable(false);

        mQuickShotSw.setOnCheckedChangeListener(this);
        mPushBackSw.setOnCheckedChangeListener(this);
        mAutoCaptureSw.setOnCheckedChangeListener(this);

        mConfirmBtn.setOnClickListener(this);
        mStopBtn.setOnClickListener(this);
        mRejectBtn.setOnClickListener(this);
        mPushDrawerIb.setOnClickListener(this);

        Spinner intervalSpinner = findViewById(R.id.spinner);
        Integer[] items = new Integer[]{4,6,8,10};
        ArrayAdapter<Integer> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, items);
        intervalSpinner.setAdapter(adapter);

        intervalSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener()
        {
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id)
            {
                photoCaptureInterval = 1000 * (int) parent.getItemAtPosition(pos);
                //writeToast("Capture interval changed to " + photoCaptureInterval);
            }
            public void onNothingSelected(AdapterView<?> parent)
            { }
        });
    }


    @Override
    public void onClick(View v) {
        if (mActiveTrackOperator == null) {
            return;
        }
        switch (v.getId()) {


            case R.id.confirm_btn:
                flag = false;
                boolean isAutoTracking =
                        isAutoSensingSupported &&
                                (mActiveTrackOperator.isAutoSensingEnabled() ||
                                        mActiveTrackOperator.isAutoSensingForQuickShotEnabled());
                if (isAutoTracking) {
                    startAutoSensingMission();
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mStopBtn.setVisibility(View.VISIBLE);
                            mRejectBtn.setVisibility(View.VISIBLE);
                            mConfirmBtn.setVisibility(View.INVISIBLE);
                        }
                    });
                } else {
                    trackingIndex = INVALID_INDEX;
                    mActiveTrackOperator.acceptConfirmation(new CommonCallbacks.CompletionCallback() {

                        @Override
                        public void onResult(DJIError error) {
                            setResultToToast(error == null ? "Accept Confirm Success!" : error.getDescription());
                        }
                    });

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mStopBtn.setVisibility(View.VISIBLE);
                            mRejectBtn.setVisibility(View.VISIBLE);
                            mConfirmBtn.setVisibility(View.INVISIBLE);
                        }
                    });

                }
                break;

            case R.id.tracking_stop_btn:
                stopTimer();
                trackingIndex = INVALID_INDEX;
                flag = true;
                mActiveTrackOperator.stopTracking(new CommonCallbacks.CompletionCallback() {

                    @Override
                    public void onResult(DJIError error) {
                        setResultToToast(error == null ? "Stop track Success!" : error.getDescription());
                    }
                });

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mTrackingImage != null) {
                            mTrackingImage.setVisibility(View.INVISIBLE);
                            //mSendRectIV.setVisibility(View.INVISIBLE);
                            mStopBtn.setVisibility(View.INVISIBLE);
                            mRejectBtn.setVisibility(View.INVISIBLE);
                            mConfirmBtn.setVisibility(View.VISIBLE);
                        }
                    }
                });
                break;

            case R.id.reject_btn:
                trackingIndex = INVALID_INDEX;
                mActiveTrackOperator.rejectConfirmation(new CommonCallbacks.CompletionCallback() {

                    @Override
                    public void onResult(DJIError error) {

                        setResultToToast(error == null ? "Reject Confirm Success!" : error.getDescription());
                    }
                });
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mStopBtn.setVisibility(View.VISIBLE);
                        mRejectBtn.setVisibility(View.VISIBLE);
                        mConfirmBtn.setVisibility(View.INVISIBLE);
                    }
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
                //writeToast("can't get retreat enable state " + error.getDescription());
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

                    writeToast("Set Retreat Enabled: " + (error == null  ?  "Success"  :  error.getDescription()));
                });
                break;
            case R.id.switchAutocapture:
                if (isChecked)
                    startPhotoCaptureTimer();
                else
                    stopPhotoCaptureTimer();
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
                    if(trackingState.getType() == ActiveTrackTargetType.HUMAN||trackingState.getType() == ActiveTrackTargetType.BIKE) {
                        flag = false;
                        trackingIndex = INVALID_INDEX;
                        setResultToToast("Tracking success");

                        mActiveTrackOperator.acceptConfirmation(new CommonCallbacks.CompletionCallback() {

                            @Override
                            public void onResult(DJIError error) {
                                setResultToToast(error == null ? "Accept Confirm Success!" : error.getDescription());
                            }
                        });

                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mStopBtn.setVisibility(View.VISIBLE);
                                mRejectBtn.setVisibility(View.VISIBLE);
                                mConfirmBtn.setVisibility(View.INVISIBLE);
                            }
                        });

                    }
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


                    /*
                    if(trackingState.getState().name()== "WAITING_FOR_CONFIRMATION"){
                       // trackingIndex = INVAVID_INDEX;
                        //mActiveTrackOperator.acceptConfirmation(null);
                        flag = false;

                    }
                    if(trackingState.getState().name()== "FINDING_TRACKED_TARGET"){
                        //mActiveTrackOperator.acceptConfirmation(null);
                        flag = false;
                    }

                    */
                //  setResultToToast("Type:" + trackingState.getType().name());

                // textScreen = findViewById(R.id.textView2);
                // runOnUiThread(()-> textScreen.setText("Type:"));
                if(trackingState.getType() == ActiveTrackTargetType.HUMAN||trackingState.getType() == ActiveTrackTargetType.BIKE) {
                    flag = false;
                    trackingIndex = INVALID_INDEX;
                    setResultToToast("Tracking success");

                    mActiveTrackOperator.acceptConfirmation(new CommonCallbacks.CompletionCallback() {

                        @Override
                        public void onResult(DJIError error) {
                            setResultToToast(error == null ? "Accept Confirm Success!" : error.getDescription());
                        }
                    });

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mStopBtn.setVisibility(View.VISIBLE);
                            mRejectBtn.setVisibility(View.VISIBLE);
                            mConfirmBtn.setVisibility(View.INVISIBLE);
                        }
                    }); mActiveTrackOperator.acceptConfirmation(new CommonCallbacks.CompletionCallback() {

                        @Override
                        public void onResult(DJIError error) {
                            setResultToToast(error == null ? "Accept Confirm Success!" : error.getDescription());
                        }
                    });

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mStopBtn.setVisibility(View.VISIBLE);
                            mRejectBtn.setVisibility(View.VISIBLE);
                            mConfirmBtn.setVisibility(View.INVISIBLE);
                        }
                    });
                        /*

                        boolean isAutoTracking =
                                isAutoSensingSupported &&
                                        (mActiveTrackOperator.isAutoSensingEnabled() ||
                                                mActiveTrackOperator.isAutoSensingForQuickShotEnabled());
                        if (isAutoTracking) {
                            startAutoSensingMission();
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    mStopBtn.setVisibility(View.VISIBLE);
                                    mRejectBtn.setVisibility(View.VISIBLE);
                                    mConfirmBtn.setVisibility(View.INVISIBLE);
                                }
                            });
                        } else {


                           // trackingIndex = INVAVID_INDEX;
                            mActiveTrackOperator.acceptConfirmation(new CompletionCallback() {

                                @Override
                                public void onResult(DJIError error) {
                                    setResultToToast(error == null ? "Accept Confirm Success!" : error.getDescription());
                                }
                            });

                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    mStopBtn.setVisibility(View.VISIBLE);
                                    mRejectBtn.setVisibility(View.VISIBLE);
                                    mConfirmBtn.setVisibility(View.INVISIBLE);
                                }
                            });

                        }
                    */
                }
                else{
                    flag = true;
                    trackingIndex = INVALID_INDEX;
                    mActiveTrackOperator.stopTracking(new CommonCallbacks.CompletionCallback() {

                        @Override
                        public void onResult(DJIError error) {
                            setResultToToast(error == null ? "Stop track Success!" : error.getDescription());
                        }
                    });

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (mTrackingImage != null) {
                                mTrackingImage.setVisibility(View.INVISIBLE);
                                //mSendRectIV.setVisibility(View.INVISIBLE);
                                mStopBtn.setVisibility(View.INVISIBLE);
                                mRejectBtn.setVisibility(View.INVISIBLE);
                                mConfirmBtn.setVisibility(View.VISIBLE);
                            }
                        }
                    });
                }
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


    // Post Result RectF       @param iv       @param rectF        @param targetState
    private void postResultRect(final ImageView iv, final RectF rectF, final ActiveTrackTargetState targetState)
    {
        View parent = (View) iv.getParent();

        final int l = (int) ((rectF.centerX() - rectF.width() / 2) * parent.getWidth());
        final int t = (int) ((rectF.centerY() - rectF.height() / 2) * parent.getHeight());
        final int r = (int) ((rectF.centerX() + rectF.width() / 2) * parent.getWidth());
        final int b = (int) ((rectF.centerY() + rectF.height() / 2) * parent.getHeight());

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

        final int l = (int) ((rectF.centerX() - rectF.width() / 2) * parent.getWidth());
        final int t = (int) ((rectF.centerY() - rectF.height() / 2) * parent.getHeight());
        final int r = (int) ((rectF.centerX() + rectF.width() / 2) * parent.getWidth());
        final int b = (int) ((rectF.centerY() + rectF.height() / 2) * parent.getHeight());

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
    private void updateMultiTrackingView(final SubjectSensingState[] targetSensingInformations)
    {
        ArrayList<Integer> indexs = new ArrayList<>();

        for (SubjectSensingState target : targetSensingInformations)
        {
            indexs.add(target.getIndex());
            if (targetViewHashMap.containsKey(target.getIndex()))
            {

                MultiTrackingView targetView = targetViewHashMap.get(target.getIndex());
                assert targetView != null;
                postMultiResultRect(targetView, target.getTargetRect(), target);
            }
            else
            {
                MultiTrackingView trackingView = new MultiTrackingView(TrackingTestActivity.this);
                mBgLayout.addView(trackingView, layoutParams);
                targetViewHashMap.put(target.getIndex(), trackingView);
            }
        }

        ArrayList<Integer> missingIndexs = new ArrayList<>();

        for (Integer key : targetViewHashMap.keySet())
        {
            boolean isDisappeared = true;
            for (Integer index : indexs)
                if (index.equals(key))
                {
                    isDisappeared = false;
                    break;
                }

            if (isDisappeared)
                missingIndexs.add(key);
        }

        for (Integer i : missingIndexs)
        {
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

                    writeToast("Set QuickShot Enabled " + (error == null ? "Success!" : error.getDescription()));
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
                writeToast(error == null ? "Disable Auto Sensing Success!" : error.getDescription());
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
                    writeToast("Accept Confirm index: " + trackingIndex + " Success!");
                    trackingIndex = INVALID_INDEX;
                }
                else
                    writeToast(error.getDescription());
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
                    writeToast("Change to SD card Success!");
                }

                @Override
                public void onFailure(@NonNull DJIError error) {
                    writeToast(error.getDescription());
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
                    writeToast("Change to Interal Storage Success!");
                }

                @Override
                public void onFailure(@NonNull DJIError error)
                {
                    writeToast(error.getDescription());
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


    // Determine Internal Storage is or not Ready        @param index        @return
    private boolean isInternalStorageReady(int index)
    {
        KeyManager keyManager = KeyManager.getInstance();

        boolean isInternalSupported = (boolean)
                keyManager.getValue(CameraKey.create(CameraKey.IS_INTERNAL_STORAGE_SUPPORTED, index));

        if (isInternalSupported)
        {
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
                        writeToast("Stop Recording Success!");
                    }

                    @Override
                    public void onFailure(@NonNull DJIError error) {
                        writeToast("Stop Recording Fail，Error " + error.getDescription());
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





    public void downloadLast(View view)     // Download last image
    {
        getFileList();

        if ( mediaFileList.size() == 0 )
        {
            writeToast( "Current list size is zero, try downloading again." );
//            writeToast( String.valueOf(mediaFileList.size()) );
            return;
        }

        mediaFileList.get(0).fetchFileData(destDir,null, new DownloadListener<String>()            // get(0) gets most recent file (with current sorting logic)
        {
            @Override
            public void onFailure(DJIError error) {
                writeToast("Download File Failed" + error.getDescription());
            }

            @Override
            public void onProgress(long total, long current)
            { }

            @Override
            public void onRateUpdate(long total, long current, long persize)
            { }

            @Override
            public void onStart() {
                writeToast("Started download.");
            }

            @Override
            public void onSuccess(String filePath) {
                writeToast("Download File Success:" + filePath);
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
                        writeToast("Get Media File List Failed:" + djiError.getDescription());
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


    private void startPhotoCaptureTimer()
    {
        photoCaptureTimer = new Timer();

        TimerTask mTt1 = new TimerTask()
        {
            public void run()
            {
                photoCaptureTimerHandler.post(() -> capturePhoto());
            }
        };

        photoCaptureTimer.schedule(mTt1, photoCaptureInterval, photoCaptureInterval);
    }


    private void stopPhotoCaptureTimer()
    {
        if ( photoCaptureTimer != null )
        {
            photoCaptureTimer.cancel();
            photoCaptureTimer.purge();
        }
    }


    private void capturePhoto()
    {
        writeToast("SNAP");

        Camera camera = getProductInstance().getCamera();

        if (camera != null)
        {
            writeToast("Camera found");

            camera.setMode(SettingsDefinitions.CameraMode.SHOOT_PHOTO, djiError ->
            {

            });

            camera.startShootPhoto(djiError ->
            {
                if (null == djiError)
                    writeToast("Captured successfully");
                else
                    writeToast(djiError.getDescription());
            });
        }
        else
            writeToast("Camera is null");

    }





    public void startStopAlertTimer()
    {
        stopAlertTimer = new Timer();

        TimerTask mTt2 = new TimerTask()
        {
            public void run()
            {
                stopAlertTimerHandler.post(() -> toggleStopAlert());
            }
        };

        stopAlertTimer.schedule(mTt2, stopAlertInterval, stopAlertInterval);
    }


    private void stopStopAlertTimer()
    {
        if ( stopAlertTimer != null )
        {
            stopAlertTimer.cancel();
            stopAlertTimer.purge();
        }
    }


    private void toggleStopAlert()
    {
        stopAlertCounter++;
        stopAlertVisible = !stopAlertVisible;
        mStopMessage.setVisibility(stopAlertVisible  ?  View.VISIBLE  :  View.INVISIBLE);

        if (stopAlertCounter == 20)
        {
            stopAlertCounter = 0;
            stopStopAlertTimer();
        }
    }




    private void ClientStatusUI(String toastMsg, String uiMsg, int uiColor)
    {
        runOnUiThread(() ->
        {
            writeToast(toastMsg);
            mClientStatus.setText(uiMsg);
            mClientStatus.setTextColor(uiColor);
        });
    }


    private String getLocalIpAddress() throws UnknownHostException
    {
        WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(WIFI_SERVICE);
        assert wifiManager != null;
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        int ipInt = wifiInfo.getIpAddress();

        return InetAddress.getByAddress(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(ipInt).array()).getHostAddress();
    }


    private void serverSetup()
    {
        try
        {
            mTextIP.setText( getLocalIpAddress() );
        }
        catch (UnknownHostException e)
        {
            e.printStackTrace();
            writeToast("Error at onCreate: " + e.getMessage());
        }

        try
        {
            serverSocket = new ServerSocket();
            serverSocket.setReuseAddress(true);
            serverSocket.bind(new InetSocketAddress(SERVER_PORT));
        }
        catch (IOException e)
        {
            e.printStackTrace();
            writeToast("Error at SocketThread: " + e.getMessage());
        }

        SocketThread = new Thread(new SocketThread());
        SocketThread.start();
    }


    class SocketThread implements Runnable
    {
        @Override
        public void run()
        {
            try
            {
                ClientStatusUI("Waiting for a new client...", "WAITING FOR CLIENT", 0xFFFFFF00 );
                clientSocket = serverSocket.accept();
                ClientStatusUI("New client found.", "CLIENT CONNECTED", 0xFF00FF00 );
                input = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                new Thread(new ReceiveThread()).start();
            }
            catch (IOException e)
            {
                e.printStackTrace();
                writeToast("Error at SocketThread: " + e.getMessage());
            }
        }
    }


    private class ReceiveThread implements Runnable
    {
        @Override
        public void run()
        {
            while (true)
            {
                try
                {
                    String message = null;

                    while (message == null)
                        message = input.readLine();


                    writeToast("Client: " + message);

                    if (message.equals("STOP"))
                        startStopAlertTimer();
                    else if (message.equals("CLOSE"))
                    {
                        clientSocket.close();
                        ClientStatusUI("Closed client connection.", "NO CLIENT", 0xFFFF0000 );
                        SocketThread = new Thread(new SocketThread());
                        SocketThread.start();
                        return;
                    }
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                    writeToast("Error at ReceiveThread: " + e.getMessage());
                }
            }
        }
    }
}

