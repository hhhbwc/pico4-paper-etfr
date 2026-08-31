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
  private static final int MAX_ATTEMPTS = 3;
  private static final String DAEMON = "/data/local/tmp/paper_bridge_daemon";
  private static final String CALIBRATE = "/data/local/tmp/calibrate_csv";
  private static final String CSV = "/data/local/tmp/paper-pico-calibration-labeled.csv";
  private static final String OUTPUT = "/data/local/tmp/paper-pico-calibration.bin";
  private final Handler handler = new Handler(Looper.getMainLooper());
  private final ExecutorService worker = Executors.newSingleThreadExecutor();
  private volatile Process activeProcess;
  private volatile boolean cancelled;
  private long runId;
  private CalibrationView view;

  @Override public void onCreate(Bundle state) {
    super.onCreate(state);
    requestWindowFeature(Window.FEATURE_NO_TITLE);
    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
    getWindow().getDecorView().setSystemUiVisibility(5894);
    final boolean restored = state != null && state.getBoolean("session_active", false);
    view = new CalibrationView(this, state == null ? 0 : state.getInt("point", 0));
    if (restored) view.setStatus("会话中断，请点击重新采集当前目标");
    setContentView(view);
  }

  @Override protected void onSaveInstanceState(Bundle out) {
    out.putInt("point", view == null ? 0 : view.point());
    out.putBoolean("session_active", view != null && view.point() < 9);
    super.onSaveInstanceState(out);
  }

  @Override protected void onDestroy() {
    cancelActiveWork();
    worker.shutdownNow();
    super.onDestroy();
  }

  @Override public void onBackPressed() {
    cancelActiveWork();
    deleteRoot(CSV);
    finish();
  }

  private void cancelActiveWork() {
    cancelled = true;
    runId++;
    handler.removeCallbacksAndMessages(null);
    Process process = activeProcess;
    if (process != null) process.destroyForcibly();
  }

  private boolean active(long token) { return !cancelled && !isFinishing() && token == runId; }

  private void startCalibration(boolean fresh) {
    if (view.running()) return;
    cancelled = false;
    final long token = ++runId;
    if (fresh) view.reset();
    view.setRunning(true);
    view.setAttempt(1);
    worker.execute(() -> {
      if (fresh) runRoot("rm -f " + CSV, 3000);
      if (!wakePaperStream(token)) {
        handler.post(() -> {
          if (active(token)) { view.setRunning(false); view.fail("Paper 视频流启动失败"); }
        });
        return;
      }
      beginPoint(token);
    });
  }

  private void beginPoint(final long token) {
    if (!active(token)) return;
    final int point = view.point();
    final int attempt = view.attempt();
    handler.post(() -> {
      if (active(token)) view.setStatus("目标 " + (point + 1) + "/9 · 尝试 " + attempt + "/" + MAX_ATTEMPTS + " · 请稳定注视");
    });
    handler.post(() -> settlePoint(token, point, attempt));
  }

  private void settlePoint(final long token, final int point, final int attempt) {
    if (!active(token) || view.point() != point || view.attempt() != attempt) return;
    view.setStatus("目标 " + (point + 1) + "/9 · 请稳定注视");
    handler.postDelayed(() -> capturePoint(token, point, attempt), 750);
  }

  private void capturePoint(final long token, final int point, final int attempt) {
    if (!active(token) || view.point() != point || view.attempt() != attempt) return;
    view.setStatus("目标 " + (point + 1) + "/9 · 正在采样");
    worker.execute(() -> {
      String command = String.format(Locale.US, "%s --target-record %d %.2f %.2f 3 %s", DAEMON, point,
          TARGET_X[point], TARGET_Y[point], CSV);
      final CommandResult result = runRoot(command, 15000);
      handler.post(() -> handleCaptureResult(token, point, attempt, result));
    });
  }

  private void handleCaptureResult(long token, int point, int attempt, CommandResult result) {
    if (!active(token) || view.point() != point || view.attempt() != attempt) return;
    if (result.code == 0) {
      if (point == 8) {
        buildArtifact(token);
      } else {
        view.advance();
        view.setAttempt(1);
        handler.postDelayed(() -> worker.execute(() -> beginPoint(token)), 250);
      }
      return;
    }
    if (attempt < MAX_ATTEMPTS) {
      view.setAttempt(attempt + 1);
      view.setStatus("目标 " + (point + 1) + "/9 · 采集失败，正在检查视频流");
      worker.execute(() -> {
        if (!wakePaperStream(token)) return;
        handler.postDelayed(() -> worker.execute(() -> beginPoint(token)), 500);
      });
      return;
    }
    view.setRunning(false);
    view.fail("目标 " + (point + 1) + " 采集失败：" + brief(result));
  }

  private boolean wakePaperStream(long token) {
    handler.post(() -> { if (active(token)) view.setStatus("检查 Paper 视频流"); });
    CommandResult ready = runRoot(DAEMON + " --wake-stream 3", 12000);
    return ready.code == 0 && active(token);
  }

  private void buildArtifact(final long token) {
    view.setStatus("九点完成，正在验证校准文件");
    worker.execute(() -> {
      CommandResult result = runRoot(CALIBRATE + " " + CSV + " " + OUTPUT, 15000);
      CommandResult size = runRoot("stat -c %s " + OUTPUT, 3000);
      final boolean valid = result.code == 0 && size.code == 0 && size.output.trim().equals("216");
      handler.post(() -> {
        if (!active(token)) return;
        view.setRunning(false);
        if (valid) view.complete(); else view.fail("校准验证失败：" + brief(result));
      });
    });
  }

  private boolean sleepWhileActive(long token, long milliseconds) {
    long deadline = System.currentTimeMillis() + milliseconds;
    while (active(token) && System.currentTimeMillis() < deadline) {
      try { Thread.sleep(Math.min(100, deadline - System.currentTimeMillis())); }
      catch (InterruptedException e) { Thread.currentThread().interrupt(); return false; }
    }
    return active(token);
  }

  private static String brief(CommandResult result) {
    String detail = result.output == null ? "" : result.output.trim().replace('\n', ' ');
    if (detail.length() > 72) detail = detail.substring(0, 72);
    return "(" + result.code + ") " + detail;
  }

  private void deleteRoot(String path) { runRoot("rm -f " + path, 3000); }

  private CommandResult runRoot(String command, long timeoutMs) {
    Process process = null;
    try {
      process = new ProcessBuilder("su", "-mm", "-c", command).redirectErrorStream(true).start();
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
    private int attempt = 1;
    private boolean running;
    private String status = "点击任意位置开始自动九点校准";
    CalibrationView(Activity context, int point) { super(context); current = Math.max(0, Math.min(8, point)); paint.setTypeface(android.graphics.Typeface.create("sans", 0)); setFocusable(true); }
    int point() { return current; }
    int attempt() { return attempt; }
    boolean running() { return running; }
    void setAttempt(int value) { attempt = value; invalidate(); }
    void setRunning(boolean value) { running = value; invalidate(); }
    void setStatus(String value) { status = value; invalidate(); }
    void advance() { current++; status = "目标 " + (current + 1) + "/9"; invalidate(); }
    void fail(String message) { status = message + "；点击重试当前目标"; invalidate(); }
    void complete() { current = 9; status = "校准完成：216 字节文件已验证；点击重新开始"; invalidate(); }
    void reset() { current = 0; attempt = 1; running = false; status = "自动九点校准"; invalidate(); }
    @Override protected void onDraw(android.graphics.Canvas canvas) {
      super.onDraw(canvas); canvas.drawColor(android.graphics.Color.BLACK); float w = getWidth(), h = getHeight();
      paint.setTextAlign(android.graphics.Paint.Align.CENTER); paint.setTextSize(Math.max(24, h * .042f)); paint.setColor(android.graphics.Color.WHITE); canvas.drawText(status, w / 2, h * .08f, paint);
      if (current < 9) { float cx = w * TARGET_X[current], cy = h * TARGET_Y[current]; paint.setColor(android.graphics.Color.WHITE); canvas.drawCircle(cx, cy, Math.max(18, h * .025f), paint); paint.setColor(android.graphics.Color.rgb(0, 216, 255)); canvas.drawCircle(cx, cy, Math.max(7, h * .010f), paint); }
      paint.setTextSize(Math.max(18, h * .03f)); paint.setColor(android.graphics.Color.LTGRAY); canvas.drawText(running ? "正在自动校准；返回键取消" : "点击任意位置开始或重试；返回键退出", w / 2, h * .94f, paint);
    }
    @Override public boolean onTouchEvent(android.view.MotionEvent event) {
      if (event.getAction() != android.view.MotionEvent.ACTION_UP || running) return true;
      startCalibration(current >= 9 || current == 0);
      return true;
    }
  }
}
