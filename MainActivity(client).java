package com.example.daonininini;

import android.content.Context;
import android.os.Bundle;
import android.os.StrictMode;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

public class MainActivity extends AppCompatActivity {

    public TextView Toptext;
    public TextView datatext;
    public TextView byText;
    public Button StartButton;
    public Button StopButton;
    public Button ConnButton;
    private Socket socket; //소켓 생성

    // fixme: TAG
    String TAG = "socketTest"; //실행 로그에서 보여주는 텍스트


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        ConnButton = findViewById(R.id.send_button); //앱 내의 버튼을 눌렀을 때의 동작을 실행하기 위한 객체 선언
        StartButton = findViewById(R.id.sensor);
        StopButton = findViewById(R.id.stop);

        final EditText ipNumber = findViewById(R.id.EditText01);

        Log.i(TAG, "Application createad");

        int SDK_INT = android.os.Build.VERSION.SDK_INT; //키보드 자동 내림
        if (SDK_INT > 8) {
            StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
            StrictMode.setThreadPolicy(policy);
        }


        ConnButton.setOnClickListener(new Button.OnClickListener() { //send_button에 리스너를 할당
            @Override
            public void onClick(View view) { //connect 버튼을 누르면 서버로 connect 함
                Toast.makeText(getApplicationContext(), "Connect 시도", Toast.LENGTH_SHORT).show();
                String addr = ipNumber.getText().toString().trim(); //ip주소 입력창의 숫자를 ip주소로 지정
                ConnectThread thread = new ConnectThread(addr);
                //connect thread 클래스를 이용해 스레드 객체를 만들 실행하는 코드를 on click 메서드 안에 넣어줌

                thread.start(); //스레드 실행 시작
            }
        });

        // fixme: 버튼 ClickListener
        StartButton.setOnClickListener(new Button.OnClickListener() { //sensor 버튼에 리스너를 할당
            @Override
            public void onClick(View view) {
                StartThread sthread = new StartThread();
                sthread.start();

            }
        });
        StopButton.setOnClickListener(new Button.OnClickListener() { //stop 버튼에 리스너를 할당
            @Override
            public void onClick(View view) {
                StopThread spthread = new StopThread();
                spthread.start();
            }
        });
    }

    class RECVThread extends Thread {

        int bytes;

        public RECVThread() { //서버 데이터 반환

            datatext = findViewById(R.id.textView2);
            byText = findViewById(R.id.textView2);
        }

        public void run() {
            try {

                //TODO:수신 데이터(프로토콜) 처리

                while (true) {
                    byte[] buffer = new byte[1024];

                    InputStream input = socket.getInputStream(); //InputStream의 read()로 서버의 데이터를 읽음

                    bytes = input.read(buffer);


                    String str = new String(buffer, 0, bytes);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            byText.setText(str);
                        }

                    });
                }
            } catch (IOException e) {

            }
        }
    }


    // fixme: Sensor on 버튼 클릭 시 분사 요청
    class StartThread extends Thread {

        public StartThread() {

            datatext = findViewById(R.id.textView2);
            byText = findViewById(R.id.textView2);
        }

        public void run() { //sensor on 버튼 클릭하면

            try {
                String OutData = "1111"; // 서버에 1111을 전송하여 분사 요청을 함
                byte[] data = OutData.getBytes();
                OutputStream output = socket.getOutputStream();
                output.write(data); //OutputStream의 write()로 데이터를 적음
                Log.d(TAG, "AT+START\\n COMMAND 송신");

            } catch (IOException e) {
                e.printStackTrace();
                Log.d(TAG, "데이터 송신 오류");
            }
        }
    }

        // fixme: Sensor off 버튼 클릭 시 분사 종료
        class StopThread extends Thread { //sensor off 버튼 클릭하면

            public void run() {

                try {
                    String OutData = "2222"; // 서버에 2222를 전송하여 분사 종료 요청을 함
                    byte[] data = OutData.getBytes();
                    OutputStream output = socket.getOutputStream();
                    output.write(data);
                    Log.d(TAG, "AT+STOP\\n COMMAND 송신");

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }


        // fixme: 소켓 연결
        class ConnectThread extends Thread { //server connect 버튼 클릭하면
            String hostname;

            public ConnectThread(String addr) {
                hostname = addr;
            } //hostname = ip주소

            public void run() {
                try { //클라이언트 소켓 생성

                    int port = 7755; // 포트 번호 설정
                    socket = new Socket(hostname, port); // 소켓 객체 생성
                    Log.d(TAG, "Socket 생성, 연결.");

                    Toptext = findViewById(R.id.text1);

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            InetAddress addr = socket.getInetAddress(); //inetaddress의 객체의 ip 주소를 반환
                            String tmp = addr.getHostAddress(); //ip 주소에 해당하는 hostname을 포함한 string을 반환
                            Toptext.setText(tmp + " 연결 완료");
                            Toast.makeText(getApplicationContext(), "Connected", Toast.LENGTH_LONG).show();

                            StartButton.setEnabled(true);
                            StopButton.setEnabled(true);

                            RECVThread recv = new RECVThread();
                            recv.start();
                        }
                    });

                        /* 오류 나면 출력할 메세지들 */
                } catch (UnknownHostException uhe) { // 소켓 생성 시 전달되는 호스트 IP 식별불가.

                    Log.e(TAG, " 생성 Error : 호스트의 IP 주소를 식별할 수 없음.(잘못된 주소 값 또는 호스트 이름 사용)");
                    runOnUiThread(new Runnable() {
                        public void run() {
                            Toast.makeText(getApplicationContext(), "Error : 호스트의 IP 주소를 식별할 수 없음.(잘못된 주소 값 또는 호스트 이름 사용)", Toast.LENGTH_SHORT).show();
                            Toptext.setText("Error : 호스트의 IP 주소를 식별할 수 없음.(잘못된 주소 값 또는 호스트 이름 사용)");
                        }
                    });

                } catch (IOException ioe) { // 소켓 생성 과정에서 I/O 에러 발생.

                    Log.e(TAG, " 생성 Error : 네트워크 응답 없음");
                    runOnUiThread(new Runnable() {
                        public void run() {
                            Toast.makeText(getApplicationContext(), "Error : 네트워크 응답 없음", Toast.LENGTH_SHORT).show();
                            Toptext.setText("네트워크 연결 오류");
                        }
                    });


                } catch (SecurityException se) { // security manager에서 허용되지 않은 기능 수행.

                    Log.e(TAG, " 생성 Error : 보안(Security) 위반에 대해 보안 관리자(Security Manager)에 의해 발생. (프록시(proxy) 접속 거부, 허용되지 않은 함수 호출)");
                    runOnUiThread(new Runnable() {
                        public void run() {
                            Toast.makeText(getApplicationContext(), "Error : 보안(Security) 위반에 대해 보안 관리자(Security Manager)에 의해 발생. (프록시(proxy) 접속 거부, 허용되지 않은 함수 호출)", Toast.LENGTH_SHORT).show();
                            Toptext.setText("Error : 보안(Security) 위반에 대해 보안 관리자(Security Manager)에 의해 발생. (프록시(proxy) 접속 거부, 허용되지 않은 함수 호출)");
                        }
                    });


                } catch (IllegalArgumentException le) { // 소켓 생성 시 전달되는 포트 번호(65536)이 허용 범위(0~65535)를 벗어남.

                    Log.e(TAG, " 생성 Error : 메서드에 잘못된 파라미터가 전달되는 경우 발생.(0~65535 범위 밖의 포트 번호 사용, null 프록시(proxy) 전달)");
                    runOnUiThread(new Runnable() {
                        public void run() {
                            Toast.makeText(getApplicationContext(), " Error : 메서드에 잘못된 파라미터가 전달되는 경우 발생.(0~65535 범위 밖의 포트 번호 사용, null 프록시(proxy) 전달)", Toast.LENGTH_SHORT).show();
                            Toptext.setText("Error : 메서드에 잘못된 파라미터가 전달되는 경우 발생.(0~65535 범위 밖의 포트 번호 사용, null 프록시(proxy) 전달)");
                        }
                    });
                }
            }
        }

        @Override
        protected void onStop() {  //앱 종료시
            super.onStop();
            try {
                socket.close(); //소켓 닫기
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


