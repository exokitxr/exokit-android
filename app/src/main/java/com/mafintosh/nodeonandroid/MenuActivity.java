/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mafintosh.nodeonandroid;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.view.View;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.JavascriptInterface;

class MenuInterface {
    Context mContext;

    /** Instantiate the interface and set the context */
    MenuInterface(Context c) {
        mContext = c;
    }

    @JavascriptInterface
    public void enterVr() {
      System.out.println("enter vr interface");

      Intent intent = new Intent(mContext, TreasureHuntActivity.class);
      /* EditText editText = (EditText) findViewById(R.id.editText);
      String message = editText.getText().toString();
      intent.putExtra(EXTRA_MESSAGE, message); */
      mContext.startActivity(intent);
    }

    @JavascriptInterface
    public void enterAr() {
      System.out.println("enter ar interface");

      Intent intent = new Intent(mContext, TreasureHuntActivity.class);
      /* EditText editText = (EditText) findViewById(R.id.editText);
      String message = editText.getText().toString();
      intent.putExtra(EXTRA_MESSAGE, message); */
      mContext.startActivity(intent);
    }
}

public class MenuActivity extends Activity {

	public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    WebView webview = new WebView(this);
    setContentView(webview);
    webview.setWebViewClient(new WebViewClient() {
      public void onReceivedError(WebView view, int errorCode, String description, String failingUrl) {
        System.out.println("webview error: " + description);
      }
    });
    // webview.setVisibility(View.GONE);
    webview.getSettings().setJavaScriptEnabled(true);
    webview.addJavascriptInterface(new MenuInterface(this), "nativeMenu");
    webview.loadUrl("file:///android_asset/public/index.html");

		/* final Context context = this;

		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		button = (Button) findViewById(R.id.buttonUrl);

		button.setOnClickListener(new OnClickListener() {

		  @Override
		  public void onClick(View arg0) {
		    Intent intent = new Intent(context, WebViewActivity.class);
		    startActivity(intent);
		  }

		}); */

	}

}
