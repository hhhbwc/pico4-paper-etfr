package com.hhhbwc.papercalibration;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class CalibrationActivity extends Activity {
  private static final float[] TARGET_X = {.10f,.50f,.90f,.10f,.50f,.90f,.10f,.50f,.90f};
  private static final float[] TARGET_Y = {.10f,.10f,.10f,.50f,.50f,.50f,.90f,.90f,.90f};
  private static final String DAEMON = "/data/local/tmp/paper_bridge_daemon";
  private static final String CALIBRATE = "/data/local/tmp/calibrate_csv";
  private static final String CSV = "/data/local/tmp/paper-pico-calibration-labeled.csv";
  private static final String OUTPUT = "/data/local/tmp/paper-pico-calibration.bin";
  private final Handler handler = new Handler(Looper.getMainLooper());
  private final ExecutorService worker = Executors.newSingleThreadExecutor();
  private volatile Process activeProcess;
  private volatile boolean cancelled;
  private CalibrationView view;

  @Override public void onCreate(Bundle state) {
    super.onCreate(state);
    requestWindowFeature(Window.FEATURE_NO_TITLE);
    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
    getWindow().getDecorView().setSystemUiVisibility(5894);
    view = new CalibrationView(this, state == null ? 0 : state.getInt("point", 0));
    setContentView(view);
  }

  @Override protected void onSaveInstanceState(Bundle out) {
    out.putInt("point", view == null ? 0 : view.point());
    super.onSaveInstanceState(out);
  }

  @Override protected void onDestroy() {
    cancelled = true;
    Process process = activeProcess;
    if (process != null) process.destroyForcibly();
    worker.shutdownNow();
    super.onDestroy();
  }

  @Override public void onBackPressed() {
    cancelled = true;
    Process process = activeProcess;
    if (process != null) process.destroyForcibly();
    worker.shutdownNow();
    deleteRoot(CSV);
    deleteRoot(OUTPUT);
    finish();
  }

  private void capturePoint(final int id) {
    view.setRunning(true);
    cancelled = false;
    worker.execute(() -> {
      if (id == 0) { runRoot("rm -f " + CSV + " " + OUTPUT, 3000); }
      handler.post(() -> startCountdown(id));
    });
  }

  private void startCountdown(final int id) {
    if (cancelled || isFinishing()) return;
    view.setStatus("保持注视 · 3");
    handler.postDelayed(() -> { if (!cancelled && !isFinishing()) { view.setStatus("保持注视 · 2"); handler.postDelayed(() -> { if (!cancelled && !isFinishing()) { view.setStatus("保持注视 · 1"); handler.postDelayed(() -> runCapture(id), 700); } }, 700); } }, 700);
  }

  private void runCapture(final int id) {
    worker.execute(() -> {
      String command = String.format(Locale.US, "%s --target-record %d %.2f %.2f 3 %s", DAEMON, id,
          TARGET_X[id], TARGET_Y[id], CSV);
      CommandResult result = runRoot(command, 15000);
      handler.post(() -> {
        if (cancelled || isFinishing()) return;
        view.setRunning(false);
        if (result.code == 0) {
          if (id == 8) buildArtifact(); else view.advance();
        } else view.fail("采集失败 (" + result.code + ")");
      });
    });
  }

  private void buildArtifact() {
    view.setRunning(true);
    worker.execute(() -> {
      CommandResult result = runRoot(CALIBRATE + " " + CSV + " " + OUTPUT, 15000);
      CommandResult size = runRoot("stat -c %s " + OUTPUT, 3000);
      final boolean valid = result.code == 0 && size.code == 0 && size.output.trim().equals("216");
      handler.post(() -> { if (cancelled || isFinishing()) return; view.setRunning(false); if (valid) view.complete(); else view.fail("校准文件生成失败"); });
    });
  }

  private void deleteRoot(String path) {
    runRoot("rm -f " + path, 3000);
  }

  private CommandResult runRoot(String command, long timeoutMs) {
    Process process = null;
    try {
      process = new ProcessBuilder("su", "-c", command).redirectErrorStream(true).start();
      activeProcess = process;
      StringBuilder output = new StringBuilder();
      BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
      long deadline = System.currentTimeMillis() + timeoutMs;
      while (process.isAlive() && System.currentTimeMillis() < deadline) {
        while (reader.ready()) output.append(reader.readLine()).append('\n');
        Thread.sleep(50);
      }
      if (process.isAlive()) { process.destroyForcibly(); return new CommandResult(124, output.toString()); }
      while (reader.ready()) output.append(reader.readLine()).append('\n');
      return new CommandResult(process.exitValue(), output.toString());
    } catch (Exception e) { if (process != null) process.destroyForcibly(); return new CommandResult(125, e.toString()); }
    finally { activeProcess = null; }
  }

  private static final class CommandResult { final int code; final String output; CommandResult(int c, String o) { code = c; output = o; } }

  private final class CalibrationView extends View {
    private final android.graphics.Paint paint = new android.graphics.Paint(3);
    private int current;
    private boolean running;
    private String status = "准备校准";
    CalibrationView(Activity context, int point) { super(context); current = Math.max(0, Math.min(8, point)); paint.setTypeface(android.graphics.Typeface.create("sans", 0)); setFocusable(true); }
    int point() { return current; }
    void setRunning(boolean value) { running = value; status = value ? "请保持注视" : status; invalidate(); }
    void setStatus(String value) { status = value; invalidate(); }
    void advance() { current++; status = "目标 " + (current + 1) + "/9"; invalidate(); }
    void fail(String message) { status = message + "，点击重试"; invalidate(); }
    void complete() { current = 9; status = "校准完成：216 字节文件已验证"; invalidate(); }
    void reset() { current = 0; running = false; status = "准备校准"; invalidate(); }
    @Override protected void onDraw(android.graphics.Canvas canvas) {
      super.onDraw(canvas); canvas.drawColor(android.graphics.Color.BLACK); float w = getWidth(), h = getHeight();
      paint.setTextAlign(android.graphics.Paint.Align.CENTER); paint.setTextSize(Math.max(24, h * .045f)); paint.setColor(android.graphics.Color.WHITE); canvas.drawText(status, w / 2, h * .08f, paint);
      if (current < 9) { float cx = w * TARGET_X[current], cy = h * TARGET_Y[current]; paint.setColor(android.graphics.Color.WHITE); canvas.drawCircle(cx, cy, Math.max(18, h * .025f), paint); paint.setColor(android.graphics.Color.rgb(0, 216, 255)); canvas.drawCircle(cx, cy, Math.max(7, h * .010f), paint); }
      paint.setTextSize(Math.max(18, h * .03f)); paint.setColor(android.graphics.Color.LTGRAY); canvas.drawText("点击目标开始，返回键退出", w / 2, h * .94f, paint);
    }
    @Override public boolean onTouchEvent(android.view.MotionEvent event) {
      if (event.getAction() != android.view.MotionEvent.ACTION_UP || running) return true;
      if (current >= 9) { reset(); return true; }
      float dx = event.getX() - getWidth() * TARGET_X[current], dy = event.getY() - getHeight() * TARGET_Y[current];
      if (dx * dx + dy * dy < Math.max(44, getHeight() * .06f) * Math.max(44, getHeight() * .06f)) capturePoint(current);
      return true;
    }
  }
}
