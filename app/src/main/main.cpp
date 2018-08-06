#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <thread>
#include <functional>

#include <v8.h>
#include <bindings.h>
#include <glfw.h>

#include <jni.h>
#include <android/log.h>
#include <ContextAndroid.h>

#ifdef OPENVR
#include <openvr-bindings.h>
#endif

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "glesjs", __VA_ARGS__))

using namespace v8;


    node::NodeService *service;
// JNIEnv *jnienv = NULL;
// jclass utilsClass;
    std::function<void()> serviceUiThreadFn;

    void queueServiceUiThread(std::function<void()> &&fn) {
        serviceUiThreadFn = fn;

        service->Scope([]() {
            serviceUiThreadFn();
        });

        std::function<void()> nopFunctoon;
        serviceUiThreadFn = nopFunctoon;
    }


// NOTE: must already be in context
    void callFunction(const char *funcname, const int argc, Local<Value> argv[]) {
        // init
        Isolate *isolate = Isolate::GetCurrent();
        Local<Context> localContext = isolate->GetCurrentContext();
        Local<Object> global = localContext->Global();

        // get function
        Local<String> jsfunc_name = String::NewFromUtf8(isolate, funcname);
        Local<Value> jsfunc_val = global->Get(jsfunc_name);
        if (!jsfunc_val->IsFunction()) return;
        Local<Function> jsfunc = Local<Function>::Cast(jsfunc_val);

        // call function, 'this' points to global object
        TryCatch try_catch(Isolate::GetCurrent());
        Local<Value> result = jsfunc->Call(global, argc, argv);

        if (result.IsEmpty()) {
            String::Utf8Value error(try_catch.Exception());
            String::Utf8Value stacktrace(try_catch.StackTrace());
            // LOGI("Error calling %s: %s:\n%s",funcname,*error,*stacktrace);
        } else {
            // LOGI("%s called",funcname);
        }
    }


    void redirectStdioToLog() {
        setvbuf(stdout, 0, _IOLBF, 0);
        setvbuf(stderr, 0, _IONBF, 0);

        int pfd[2];
        pipe(pfd);
        dup2(pfd[1], 1);
        dup2(pfd[1], 2);

        std::thread([](int pfd0) {
            char buf[1024];
            std::size_t nBytes = 0;
            while ((nBytes = read(pfd0, buf, sizeof buf - 1)) > 0) {
                if (buf[nBytes - 1] == '\n') --nBytes;
                buf[nBytes] = 0;
                LOGI("%s", buf);
            }
        }, pfd[0]).detach();
    }


#ifdef __cplusplus
    extern "C" {
#endif

    JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_onResize
            (JNIEnv *env, jclass clas, jint width, jint height) {
        LOGI("JNI onResize %d %d", width, height);

        {
            HandleScope handle_scope(Isolate::GetCurrent());

            unsigned int width = 1;
            unsigned int height = 1;

            Handle<Number> js_width = v8::Integer::New(Isolate::GetCurrent(), width);
            Handle<Number> js_height = v8::Integer::New(Isolate::GetCurrent(), height);

            Local<Value> argv[] = {js_width, js_height};
            callFunction("onResize", sizeof(argv) / sizeof(argv[0]), argv);
        }
    }


    JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_onNewFrame
            (JNIEnv *env, jclass clas, jfloatArray headViewMatrix, jfloatArray headQuaternion,
             jfloatArray centerArray) {
        float headViewMatrixElements[] = {0};
        float headQuaternionElements[] = {0};
        float centerArrayElements[] = {0};

        {
            HandleScope handle_scope(Isolate::GetCurrent());

            Local<Float32Array> headMatrixFloat32Array = Float32Array::New(
                    ArrayBuffer::New(Isolate::GetCurrent(), 16 * 4), 0, 16);
            for (int i = 0; i < 16; i++) {
                headMatrixFloat32Array->Set(i, Number::New(Isolate::GetCurrent(),
                                                           headViewMatrixElements[i]));
            }
            Local<Float32Array> headQuaternionFloat32Array = Float32Array::New(
                    ArrayBuffer::New(Isolate::GetCurrent(), 4 * 4), 0, 4);
            for (int i = 0; i < 4; i++) {
                headQuaternionFloat32Array->Set(i, Number::New(Isolate::GetCurrent(),
                                                               headQuaternionElements[i]));
            }
            Local<Float32Array> centerFloat32Array = Float32Array::New(
                    ArrayBuffer::New(Isolate::GetCurrent(), 3 * 4), 0, 3);
            for (int i = 0; i < 3; i++) {
                centerFloat32Array->Set(i,
                                        Number::New(Isolate::GetCurrent(), centerArrayElements[i]));
            }
            Local<Value> argv[] = {headMatrixFloat32Array, headQuaternionFloat32Array,
                                   centerFloat32Array};
            callFunction("onNewFrame", sizeof(argv) / sizeof(argv[0]), argv);
        }
    }


    JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_onDrawEye
            (JNIEnv *env, jclass clasj, jfloatArray eyeViewMatrix,
             jfloatArray eyePerspectiveMatrix) {
        float eyeViewMatrixElements[] = {0};
        float eyePerspectiveMatrixElements[] = {0};

        {
            HandleScope handle_scope(Isolate::GetCurrent());

            Local<Float32Array> eyeViewMatrixFloat32Array = Float32Array::New(
                    ArrayBuffer::New(Isolate::GetCurrent(), 16 * 4), 0, 16);
            for (int i = 0; i < 16; i++) {
                eyeViewMatrixFloat32Array->Set(i, Number::New(Isolate::GetCurrent(),
                                                              eyeViewMatrixElements[i]));
            }
            Local<Float32Array> eyePerspectiveMatrixFloat32Array = Float32Array::New(
                    ArrayBuffer::New(Isolate::GetCurrent(), 4 * 4), 0, 4);
            for (int i = 0; i < 4; i++) {
                eyePerspectiveMatrixFloat32Array->Set(i, Number::New(Isolate::GetCurrent(),
                                                                     eyePerspectiveMatrixElements[i]));
            }
            Local<Value> argv[] = {eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array};
            callFunction("onDrawEye", sizeof(argv) / sizeof(argv[0]), argv);
        }
    }


    JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_onDrawFrame
            (JNIEnv *env, jclass clas, jfloatArray viewMatrix, jfloatArray projectionMatrix,
             jfloatArray centerArray) {
        float viewMatrixElements[] = {0};
        float projectionMatrixElements[] = {0};
        float centerArrayElements[] = {0};

        {
            HandleScope handle_scope(Isolate::GetCurrent());

            Local<Float32Array> viewFloat32Array = Float32Array::New(
                    ArrayBuffer::New(Isolate::GetCurrent(), 16 * 4), 0, 16);
            for (int i = 0; i < 16; i++) {
                viewFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), viewMatrixElements[i]));
            }
            Local<Float32Array> projectionFloat32Array = Float32Array::New(
                    ArrayBuffer::New(Isolate::GetCurrent(), 16 * 4), 0, 16);
            for (int i = 0; i < 16; i++) {
                projectionFloat32Array->Set(i, Number::New(Isolate::GetCurrent(),
                                                           projectionMatrixElements[i]));
            }
            Local<Float32Array> centerFloat32Array = Float32Array::New(
                    ArrayBuffer::New(Isolate::GetCurrent(), 3 * 4), 0, 3);
            for (int i = 0; i < 3; i++) {
                centerFloat32Array->Set(i,
                                        Number::New(Isolate::GetCurrent(), centerArrayElements[i]));
            }
            Local<Value> argv[] = {viewFloat32Array, projectionFloat32Array, centerFloat32Array};
            callFunction("onDrawFrame", sizeof(argv) / sizeof(argv[0]), argv);
        }
    }

    std::function<void(node::NodeService *nodeService)> nodeServiceInitFunction;
    JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_start
            (JNIEnv *env, jobject thiz, jstring binPath, jstring jsPath, jstring libpath,
             jobject assetManager, jstring url, jstring vrMode, jint vrTexture) {
        redirectStdioToLog();

        /* AAssetManager *aAssetManager = AAssetManager_fromJava(env, assetManager);
        canvas::AndroidContextFactory *canvasContextFactory = new canvas::AndroidContextFactory(aAssetManager, 1); */
        canvas::AndroidContextFactory::initialize(env, assetManager);
        canvas::AndroidContextFactory *canvasContextFactory = new canvas::AndroidContextFactory(
                nullptr, 1);
//        CanvasRenderingContext2D::InitalizeStatic(canvasContextFactory);

        const char *binPathString = env->GetStringUTFChars(binPath, NULL);
        const char *jsPathString = env->GetStringUTFChars(jsPath, NULL);
        const char *libPathString = env->GetStringUTFChars(libpath, NULL);
        const char *urlString = env->GetStringUTFChars(url, NULL);
        const char *vrModeString = env->GetStringUTFChars(vrMode, NULL);
        std::stringstream vrTextureStringStream;
        vrTextureStringStream << vrTexture;
        const char *vrTextureString = vrTextureStringStream.str().c_str();

        char argsString[4096];
        int i = 0;

        char *binPathArg = argsString + i;
        strncpy(binPathArg, binPathString, sizeof(argsString) - i);
        i += strlen(binPathString) + 1;

        char *jsPathArg = argsString + i;
        strncpy(jsPathArg, jsPathString, sizeof(argsString) - i);
        i += strlen(jsPathString) + 1;

        char *libPathArg = argsString + i;
        strncpy(libPathArg, libPathString, sizeof(argsString) - i);
        i += strlen(libPathString) + 1;

        char *urlArg = argsString + i;
        strncpy(urlArg, urlString, sizeof(argsString) - i);
        i += strlen(urlString) + 1;

        char *vrModeArg = argsString + i;
        strncpy(vrModeArg, vrModeString, sizeof(argsString) - i);
        i += strlen(vrModeString) + 1;

        char *vrTextureArg = argsString + i;
        strncpy(vrTextureArg, vrTextureString, sizeof(argsString) - i);
        i += strlen(vrTextureString) + 1;

        char *args[] = {binPathArg, jsPathArg, libPathArg, urlArg, vrModeArg, vrTextureArg};

        node::Start(3, args);
        //service = new node::NodeService(3, args);

        nodeServiceInitFunction = [&](node::NodeService *service) {
            Isolate *isolate = service->GetIsolate();
            Local<Object> global = service->GetContext()->Global();

//            std::pair<Local<Value>, Local<FunctionTemplate>> glResult = makeGl();
//            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGl"), glResult.first);
//
//            std::pair<Local<Value>, Local<FunctionTemplate>> gl2Result = makeGl2();
//            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGl2"), gl2Result.first);

            Local<Value> image = makeImage();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImage"), image);

            Local<Value> imageData = makeImageData();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImageData"),
                         imageData);

            Local<Value> imageBitmap = makeImageBitmap();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImageBitmap"),
                         imageBitmap);

            Local<Value> path2d = makePath2D();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativePath2D"), path2d);

            Local<Value> canvasGradient = makeCanvasGradient();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCanvasGradient"),
                         canvasGradient);

            Local<Value> canvasPattern = makeCanvasPattern();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCanvasPattern"),
                         canvasPattern);

            Local<Value> canvas = makeCanvasRenderingContext2D(imageData, canvasGradient,
                                                               canvasPattern);
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(),
                                                 "nativeCanvasRenderingContext2D"), canvas);

            Local<Value> audio = makeAudio();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeAudio"), audio);

            Local<Value> video = makeVideo(imageData);
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeVideo"), video);

            /* Local<Value> glfw = makeGlfw();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGlfw"), glfw); */

            Local<Value> window = makeWindow();
            global->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeWindow"), window);
        };
        service = new node::NodeService(sizeof(args) / sizeof(args[0]), args,
                                        [](node::NodeService *service) {
                                            nodeServiceInitFunction(service);
                                        });

        std::function<void(node::NodeService *nodeService)> nopFunction;
        nodeServiceInitFunction = nopFunction;
    }

    JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_tick
            (JNIEnv *env, jobject thiz, jint timeout) {
        service->Tick(timeout);
    }

//    void InitExports(Handle<Object> exports) {
//        std::pair<Local<Value>, Local<FunctionTemplate>> glResult = makeGl();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGl"), glResult.first);
//
//        std::pair<Local<Value>, Local<FunctionTemplate>> gl2Result = makeGl2(glResult.second);
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGl2"), gl2Result.first);
//
//        Local<Value> image = makeImage();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImage"), image);
//
//        Local<Value> imageData = makeImageData();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImageData"), imageData);
//
//        Local<Value> imageBitmap = makeImageBitmap();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImageBitmap"), imageBitmap);
//
//        Local<Value> path2d = makePath2D();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativePath2D"), path2d);
//
//        Local<Value> canvasGradient = makeCanvasGradient();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCanvasGradient"), canvasGradient);
//
//        Local<Value> canvasPattern = makeCanvasPattern();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCanvasPattern"), canvasPattern);
//
//        Local<Value> canvas = makeCanvasRenderingContext2D(imageData, canvasGradient, canvasPattern);
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCanvasRenderingContext2D"), canvas);
//
//        Local<Value> audio = makeAudio();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeAudio"), audio);
//
//        Local<Value> video = makeVideo(imageData);
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeVideo"), video);
//
//        /* Local<Value> glfw = makeGlfw();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGlfw"), glfw); */
//
//        Local<Value> window = makeWindow();
//        exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeWindow"), window);
//
//
//        uintptr_t initFunctionAddress = (uintptr_t)InitExports;
//        Local<Array> initFunctionAddressArray = Nan::New<Array>(2);
//        initFunctionAddressArray->Set(0, Nan::New<Integer>((uint32_t)(initFunctionAddress >> 32)));
//        initFunctionAddressArray->Set(1, Nan::New<Integer>((uint32_t)(initFunctionAddress & 0xFFFFFFFF)));
//        exports->Set(JS_STR("initFunctionAddress"), initFunctionAddressArray);
//    }



#ifdef __cplusplus
    }
#endif
