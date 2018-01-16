// (c) 2014 Boris van Schooten
//BEGIN_INCLUDE(all)
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <functional>

#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
// at least some defs from gl1 are needed
#include <GLES/gl.h>
#include <GLES2/gl2.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "glesjs", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "glesjs", __VA_ARGS__))

#include <android/asset_manager.h>


#include <libplatform/libplatform.h>
#include <v8.h>
#include <node.h>

using namespace v8;

/* // http://engineering.prezi.com/blog/2013/08/27/embedding-v8/
class MallocArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
  public:
	virtual void* Allocate(size_t length) {
		return calloc(length, 1);
	}
	virtual void* AllocateUninitialized(size_t length) {
		return malloc(length);
	}
  virtual void* Reserve(size_t length) {
		return malloc(length);
  }
	// XXX we assume length is not needed
	virtual void Free(void*data, size_t length) {
		free(data);
	}
  virtual void Free(void*data, size_t length, v8::ArrayBuffer::Allocator::AllocationMode allocationMode) {
		free(data);
	}
  virtual void SetProtection(void *data, size_t length, v8::ArrayBuffer::Allocator::Protection protection) {
    // nothing
  }
}; */

//static void enableTypedArrays() {
//  v8::internal::FLAG_harmony_array_buffer = true;
//  v8::internal::FLAG_harmony_typed_arrays = true;
//  V8::SetArrayBufferAllocator(new MallocArrayBufferAllocator());
//}


// -----------------------------------------------
// data defs
// -----------------------------------------------
// -----------------------------------------------
// globals
// -----------------------------------------------

long readAsset(const char *filename, char **output);


int32_t screenwidth;
int32_t screenheight;
node::NodeService *service;


class JsEnvironment {
  public:
	v8::Persistent<Context> context;
	v8::Handle<v8::ObjectTemplate> global;
	v8::Isolate *isolate;
	JsEnvironment();
	char *run_javascript(char *sourcestr);
	void callFunction(const char *funcname,const int argc,Local<Value> argv[]);
};

JsEnvironment *js=NULL;

JNIEnv *jnienv = NULL;

jclass utilsClass;

#define NR_PLAYERS 4
#define NR_BUTTONS 17
#define NR_AXES 6
#define PLAYERDATASIZE  (1+NR_BUTTONS+NR_AXES)

// static array with all gamepad data
float gamepadvalues[NR_PLAYERS*PLAYERDATASIZE];

// utils

/* // String data = loadStringAsset(String filename)
void __utils_loadStringAsset(const v8::FunctionCallbackInfo<v8::Value>& args) {
	String::Utf8Value _str_assetname(args[0]->ToString(args.GetIsolate()));
	const char *assetname = *_str_assetname;
	char *data;
	readAsset(assetname,&data);
	LOGI("Loaded string asset '%s'",assetname);
	args.GetReturnValue().Set(v8::String::NewFromUtf8(args.GetIsolate(),
		data));
}

// void execScript(String source)
void __utils_execScript(const v8::FunctionCallbackInfo<v8::Value>& args) {
	String::Utf8Value _str_source(args[0]->ToString(args.GetIsolate()));
	char *source = *_str_source;
	char *ret = js->run_javascript(source);
	LOGI("Executed JS");
} */


// localStorage

void __localStorage_getItem(const v8::FunctionCallbackInfo<v8::Value>& args) {
	String::Utf8Value _str_key(args[0]->ToString(args.GetIsolate()));
	const char *key = *_str_key;

	jmethodID mid = jnienv->GetStaticMethodID(utilsClass, "storeGetString",
		"(Ljava/lang/String;)Ljava/lang/String;");
	jstring jnikey = jnienv->NewStringUTF(key);
	jstring jniret = (jstring)
		jnienv->CallStaticObjectMethod(utilsClass,mid, jnikey);
	jnienv->DeleteLocalRef(jnikey);
	if (jniret==NULL) return; // undefined
	const char *ret = jnienv->GetStringUTFChars(jniret, 0);
	args.GetReturnValue().Set(v8::String::NewFromUtf8(args.GetIsolate(), ret));
	jnienv->ReleaseStringUTFChars(jniret, ret);
	// not sure if it's necessary
	jnienv->DeleteLocalRef(jniret);
}

void __localStorage_setItem(const v8::FunctionCallbackInfo<v8::Value>& args) {
	String::Utf8Value _str_key(args[0]->ToString(args.GetIsolate()));
	const char *key = *_str_key;
	String::Utf8Value _str_val(args[1]->ToString(args.GetIsolate()));
	const char *val = *_str_val;

	jmethodID mid = jnienv->GetStaticMethodID(utilsClass, "storeSetString",
		"(Ljava/lang/String;Ljava/lang/String;)V");
	jstring jnikey = jnienv->NewStringUTF(key);
	jstring jnival = jnienv->NewStringUTF(val);
	jnienv->CallStaticVoidMethod(utilsClass,mid, jnikey,jnival);
	jnienv->DeleteLocalRef(jnikey);
	jnienv->DeleteLocalRef(jnival);
}

void __localStorage_removeItem(const v8::FunctionCallbackInfo<v8::Value>& args){
	String::Utf8Value _str_key(args[0]->ToString(args.GetIsolate()));
	const char *key = *_str_key;

	jmethodID mid = jnienv->GetStaticMethodID(utilsClass, "storeRemove",
		"(Ljava/lang/String;)V");
	jstring jnikey = jnienv->NewStringUTF(key);
	jnienv->CallStaticVoidMethod(utilsClass,mid, jnikey);
	jnienv->DeleteLocalRef(jnikey);
}

// audio

// op - LOAD,PLAY,PAUSE (int)
// assetname - String
// loop - boolean
// id - int
void __audio_handle(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int op = (int)args[0]->IntegerValue();
	String::Utf8Value _str_assetname(args[1]->ToString(args.GetIsolate()));
	const char *assetname = *_str_assetname;
	int loop = (int)args[2]->IntegerValue();
	int id = (int)args[3]->IntegerValue();
	//LOGI("audio handle %d %s %d %d",op,assetname,loop,id);
	// call method:
  	//static void handleAudio(int, java.lang.String, boolean, int);
    //Signature: (ILjava/lang/String;ZI)V
	jmethodID mid = jnienv->GetStaticMethodID(utilsClass, "handleAudio",
		"(ILjava/lang/String;ZI)V");
	jstring jniassetname = jnienv->NewStringUTF(assetname);
	jnienv->CallStaticVoidMethod(utilsClass,mid, (jint)op,jniassetname,
		(jboolean)loop, (jint)id);
	jnienv->DeleteLocalRef(jniassetname);
}

// gl

// internal functions used by glbinding


#define UNIFORMINT   0
#define UNIFORMFLOAT 1
#define UNIFORMMATRIXFLOAT 2
// vecsize: 1,2,3,4 (for matrices: 2,3,4)
// type: 0 = integer  1 = float  2 = float matrix
// transpose: only relevant for matrices
// uniform:
// args[0]: location
// args[1]: data array (floats or ints)
// uniformmatrix:
// args[0]: location
// args[1]: transpose
// args[2]: data array (floats)
void __uniformv(const v8::FunctionCallbackInfo<v8::Value>& args,int vecsize,
int type) {
	HandleScope handle_scope(js->isolate);
	GLint location = (unsigned int)args[0]->IntegerValue();
	GLboolean transpose=false;
	int dataidx = 1;
	if (type==UNIFORMMATRIXFLOAT) {
		transpose = args[1]->BooleanValue();
		dataidx = 2;
	}
	GLsizei count;
	int32_t *data;
	//https://github.com/inh3/node-threads/blob/master/src/node-threads/web-worker.cc
	if (args[dataidx]->IsArrayBufferView() || args[dataidx]->IsArrayBuffer()) {
		// typed array
		v8::Handle<v8::ArrayBufferView> bufview_data = Handle<ArrayBufferView>::Cast(args[dataidx]);
		v8::Handle<v8::ArrayBuffer> buf_data = bufview_data->Buffer();
		v8::ArrayBuffer::Contents con_data=buf_data->GetContents();
		count = con_data.ByteLength() / 4; // should always be multiple of 4
		data = (int32_t *)con_data.Data();
	} else if (args[dataidx]->IsArray()) {
		// regular array
		v8::Handle<v8::Array> array_data = Handle<Array>::Cast(args[dataidx]);
		count = array_data->Length();
		data = (int32_t *)malloc(count * 4); // float and int32 both 4 bytes
		if (type==UNIFORMINT) {
			int32_t *data_p = data;
			for (int i=0; i<count; i++) {
				v8::Handle<v8::Value> value = array_data->Get(i);
				*data_p = value->Int32Value();
				data_p++;
			}
		} else { // UNIFORMFLOAT, UNIFORMMATRIXFLOAT
			GLfloat *data_p = (GLfloat *)data;
			for (int i=0; i<count; i++) {
				v8::Handle<v8::Value> value = array_data->Get(i);
				*data_p = (GLfloat)value->NumberValue();
				//LOGI("### GOT VAL: %f",*data_p);
				data_p++;
			}
		}
	}
	//LOGI("#### Entered Uniformv %d,%d /%d,%d",vecsize, type, location, count);
	//GLfloat *dataf = (GLfloat *)data;
	//LOGI("### data val: %f %f %f %f",dataf[0],dataf[1],dataf[2],dataf[3]);
	switch (type + 3*vecsize) {
		case UNIFORMINT+1*3:
			glUniform1iv(location, count, data);
		break;
		case UNIFORMINT+2*3:
			glUniform2iv(location, count/2, data);
		break;
		case UNIFORMINT+3*3:
			glUniform3iv(location, count/3, data);
		break;
		case UNIFORMINT+4*3:
			glUniform4iv(location, count/4, data);
		break;
		case UNIFORMFLOAT+1*3:
			glUniform1fv(location, count, (const GLfloat *)data);
		break;
		case UNIFORMFLOAT+2*3:
			glUniform2fv(location, count/2, (const GLfloat *)data);
		break;
		case UNIFORMFLOAT+3*3:
			glUniform3fv(location, count/3, (const GLfloat *)data);
		break;
		case UNIFORMFLOAT+4*3:
			glUniform4fv(location, count/4, (const GLfloat *)data);
			//LOGI("### data val: %f %f %f %f",dataf[0],dataf[1],dataf[2],dataf[3]);
		break;
		case UNIFORMMATRIXFLOAT+2*3:
			glUniformMatrix2fv(location, count/(2*2), transpose,
				(const GLfloat *)data);
		break;
		case UNIFORMMATRIXFLOAT+3*3:
			glUniformMatrix3fv(location, count/(3*3), transpose,
				(const GLfloat *)data);
		break;
		case UNIFORMMATRIXFLOAT+4*3:
			glUniformMatrix4fv(location, count/(4*4), transpose,
				(const GLfloat *)data);
		break;
		default:
			LOGI("uniformv error: illegal type combination %d,%d",type,vecsize);
	}
}

// read generated bindings

#include "gluegen/glbindings.h"

// manually coded functions

void __createBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	GLuint buffers[1];
	glGenBuffers(1,buffers);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), buffers[0]));
}

void __createRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	GLuint buffers[1];
	glGenFramebuffers(1,buffers);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), buffers[0]));
}

void __createFramebuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	GLuint buffers[1];
	glGenFramebuffers(1,buffers);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), buffers[0]));
}

void __createTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
	GLuint textures[1];
	glGenTextures(1,textures);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), textures[0]));
}



void __getProgramParameter(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int param[1];
	GLuint program = (GLuint)args[0]->IntegerValue();
	GLenum pname = (GLenum)args[1]->IntegerValue();
	glGetShaderiv(program,pname,param);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), param[0]));
}

void __getShaderParameter(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int param[1];
	GLuint shader = (GLuint)args[0]->IntegerValue();
	GLenum pname = (GLenum)args[1]->IntegerValue();
	glGetShaderiv(shader,pname,param);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), param[0]));
}

void __getProgramInfoLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
	GLuint program = (GLuint)args[0]->IntegerValue();
	int length[1];
	GLchar infolog[256];
	// we can use glGetProgramiv to get the precise length of the string
	// beforehand
	glGetShaderInfoLog(program,256,length,infolog);
	args.GetReturnValue().Set(v8::String::NewFromUtf8(args.GetIsolate(),
		infolog));
}

void __getShaderInfoLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
	GLuint shader = (GLuint)args[0]->IntegerValue();
	int length[1];
	GLchar infolog[256];
	// we can use glGetShaderiv to get the precise length of the string
	// beforehand
	glGetShaderInfoLog(shader,256,length,infolog);
	args.GetReturnValue().Set(v8::String::NewFromUtf8(args.GetIsolate(),
		infolog));
}

// Trilingual binding JS -> C++ -> Java.  
//jni docs:
//http://www3.ntu.edu.sg/home/ehchua/programming/java/JavaNativeInterface.html

// We use Java's texImage2D because it parses pngs and jpgs properly.
// No NDK equivalent exists.
// We use GLUtils, which provides (amongst others):
//   texImage2D(target,level,image,border);
// We use defaults for format, internalformat, type.
void __texImage2D(const v8::FunctionCallbackInfo<v8::Value>& args) {
	// call method:
	// (get signature with javap -s [classfile])
	// static void Test()
	//jmethodID mid = jnienv->GetStaticMethodID(utilsClass, "Test", "()V");
	//jnienv->CallStaticVoidMethod(utilsClass,mid);

	// call method:
	// static void texImage2D(int target,int level,byte [] data,int border)
	jmethodID mid = jnienv->GetStaticMethodID(utilsClass, "texImage2D",
		"(II[BI)[I");
	// get js parameters
	int target = (int)args[0]->Int32Value();
	int level = (int)args[1]->Int32Value();
	if (args.Length()>=8) {
		// long version: texImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, ArrayBufferView? pixels)
		GLenum internalformat = (int)args[2]->Int32Value();
		GLsizei width = (int)args[3]->Int32Value();
		GLsizei height = (int)args[4]->Int32Value();
		GLint border = (int)args[5]->Int32Value();
		GLenum format = (int)args[6]->Int32Value();
		GLenum type = (int)args[7]->Int32Value();
		// data may be null in the long version, in which case an empty image
		// is created
		GLvoid *pixels;
		if (args.Length()>8 && !(args[8]->IsNull())) {
			// TODO get pixels from ArrayBufferView
		} else {
			// clear buffer. WebGL apparently expects the render texture to be
			// empty, while Opengl will have garbage in the buffer when not
			// explicity cleared
			pixels = (void *)calloc(4,width*height);
		}
		glTexImage2D(target,level,internalformat,width,height,border,format,
			type,pixels);
	}
	// short version:texImage2D(target,level,format,internalformat,type,image);
	// Not sure if border parameter is supported
	// and where it should go.
	String::Utf8Value _str_assetname(args[5]->ToString(args.GetIsolate()));
	const char *assetname = *_str_assetname;
	// read asset.  XXX move this to Java
	char *imagedata;
	int imagedatalen = readAsset(assetname,&imagedata);
	// convert parameters to java
    jbyteArray jBuff = jnienv->NewByteArray(imagedatalen);
    jnienv->SetByteArrayRegion(jBuff, 0, imagedatalen, (jbyte*) imagedata);
	// docs for byte array functions:
	//jbyteArray    NewByteArray(JNIEnv *env, jsize length);
	//void      ReleaseByteArrayElements(JNIEnv *env, jbyteArray array, jbyte *elems, jint mode);
	//void GetByteArrayRegion(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
	//void SetByteArrayRegion(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
	jintArray retval_j = (jintArray) jnienv->CallStaticObjectMethod(
		utilsClass,mid, (jint)target, (jint)level, jBuff, (jint)0);
	//http://www.rgagnon.com/javadetails/java-0287.html
	jint *retval = jnienv->GetIntArrayElements(retval_j, 0);
	int retwidth = retval[0];
	int retheight = retval[1];
	jnienv->ReleaseIntArrayElements(retval_j, retval, 0);//XXX use JNI_ABORT
	jnienv->DeleteLocalRef(retval_j);
	jnienv->DeleteLocalRef(jBuff);
	// XXX return value no longer used!
	// return width, height
	// from: https://v8.googlecode.com/svn/trunk/test/cctest/test-api.cc
	v8::Handle<v8::Array> jsretval =
		v8::Array::New(args.GetIsolate(), 2);
	jsretval->Set(0, v8::Integer::New(args.GetIsolate(), retwidth));
	jsretval->Set(1, v8::Integer::New(args.GetIsolate(), retheight));
	args.GetReturnValue().Set(jsretval);
}


void __getImageDimensions(const v8::FunctionCallbackInfo<v8::Value>& args) {
	String::Utf8Value _str_assetname(args[0]->ToString(args.GetIsolate()));
	const char *assetname = *_str_assetname;
	jstring jniassetname = jnienv->NewStringUTF(assetname);
	jmethodID mid = jnienv->GetStaticMethodID(utilsClass, "getImageDimensions",
		"(Ljava/lang/String;)[I");
	jintArray retval_j = (jintArray) jnienv->CallStaticObjectMethod(
		utilsClass, mid, jniassetname);
	//http://www.rgagnon.com/javadetails/java-0287.html
	jint *retval = jnienv->GetIntArrayElements(retval_j, 0);
	int retwidth = retval[0];
	int retheight = retval[1];
	jnienv->ReleaseIntArrayElements(retval_j, retval, 0);//XXX use JNI_ABORT
	jnienv->DeleteLocalRef(retval_j);
	jnienv->DeleteLocalRef(jniassetname);
	// return width, height
	// from: https://v8.googlecode.com/svn/trunk/test/cctest/test-api.cc
	v8::Handle<v8::Array> jsretval =
		v8::Array::New(args.GetIsolate(), 2);
	jsretval->Set(0, v8::Integer::New(args.GetIsolate(), retwidth));
	jsretval->Set(1, v8::Integer::New(args.GetIsolate(), retheight));
	args.GetReturnValue().Set(jsretval);
}

void __getWindowWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),
		screenwidth));
}

void __getWindowHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),
		screenheight));
}

std::function<void ()> scopeFn;

void scope(std::function<void ()> fn) {
  scopeFn = fn;

  service->Scope([]() {
    scopeFn();
  });
}

// NOTE: must already be in context
void callFunction(const char *funcname, const int argc, Local<Value> argv[]) {
  // init
  Isolate *isolate = service->GetIsolate();
  Local<Context> localContext = service->GetContext();
  Local<Object> global = localContext->Global();

  // get function
  Local<String> jsfunc_name = String::NewFromUtf8(isolate,funcname);
  Local<Value> jsfunc_val = global->Get(jsfunc_name);
  if (!jsfunc_val->IsFunction()) return;
  Local<Function> jsfunc = Local<Function>::Cast(jsfunc_val);

  // call function, 'this' points to global object
  TryCatch try_catch;
  Local<Value> result = jsfunc->Call(global, argc, argv);

  if (result.IsEmpty()) {
    String::Utf8Value error(try_catch.Exception());
    String::Utf8Value stacktrace(try_catch.StackTrace());
    LOGI("Error calling %s: %s:\n%s",funcname,*error,*stacktrace);
  } else {
    //LOGI("%s called",funcname);
  }
}


// -----------------------------------------------
// JS handling
// -----------------------------------------------





// output must be freed using free()
long readAsset(const char *filename, char **output) {
	LOGI("readAsset %s",filename);
	jmethodID mid = jnienv->GetStaticMethodID(utilsClass, "readAsset",
		"(Ljava/lang/String;)[B");
	jstring jAssetName = jnienv->NewStringUTF(filename);
	jbyteArray retval_j = (jbyteArray)
		jnienv->CallStaticObjectMethod(utilsClass, mid, jAssetName);
	jbyte* retval = jnienv->GetByteArrayElements(retval_j, 0);
	long retval_len = jnienv->GetArrayLength(retval_j);
	// allocate one zero guard byte to ensure strings are terminated
    char* buffer = (char*) calloc (sizeof(char)*(retval_len+1),1);
	memcpy(buffer,retval,retval_len);
	jnienv->ReleaseByteArrayElements(retval_j, retval, JNI_ABORT);
	jnienv->DeleteLocalRef(jAssetName);
	jnienv->DeleteLocalRef(retval_j);
	*output = buffer;
	return retval_len;
	
}

// jni interface


jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
		return -1;
	}

	// Get jclass with env->FindClass.
	// Register methods with env->RegisterNatives.

	return JNI_VERSION_1_6;
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

v8::Local<v8::Object> makeGl() {
  Isolate *isolate = service->GetIsolate();
  v8::Local<v8::ObjectTemplate> _gl = v8::ObjectTemplate::New(isolate);

	_gl->Set(v8::String::NewFromUtf8(isolate, "createRenderbuffer"),
			v8::FunctionTemplate::New(isolate, __createRenderbuffer));

	_gl->Set(v8::String::NewFromUtf8(isolate, "createFramebuffer"),
			v8::FunctionTemplate::New(isolate, __createFramebuffer));

	_gl->Set(v8::String::NewFromUtf8(isolate, "createBuffer"),
			v8::FunctionTemplate::New(isolate, __createBuffer));

	_gl->Set(v8::String::NewFromUtf8(isolate, "createTexture"),
			v8::FunctionTemplate::New(isolate, __createTexture));

	_gl->Set(v8::String::NewFromUtf8(isolate, "getProgramParameter"),
			v8::FunctionTemplate::New(isolate, __getProgramParameter));

	_gl->Set(v8::String::NewFromUtf8(isolate, "getShaderParameter"),
			v8::FunctionTemplate::New(isolate, __getShaderParameter));

	_gl->Set(v8::String::NewFromUtf8(isolate, "getProgramInfoLog"),
			v8::FunctionTemplate::New(isolate, __getProgramInfoLog));

	_gl->Set(v8::String::NewFromUtf8(isolate, "getShaderInfoLog"),
			v8::FunctionTemplate::New(isolate, __getShaderInfoLog));

	// the "raw" function takes an asset location string.
	// html5.js provides a wrapper that takes Image
	_gl->Set(v8::String::NewFromUtf8(isolate, "_texImage2D"),
			v8::FunctionTemplate::New(isolate, __texImage2D));

	_gl->Set(v8::String::NewFromUtf8(isolate, "_getImageDimensions"),
			v8::FunctionTemplate::New(isolate, __getImageDimensions));

	_gl->Set(v8::String::NewFromUtf8(isolate, "_getWindowWidth"),
			v8::FunctionTemplate::New(isolate, __getWindowWidth));

	_gl->Set(v8::String::NewFromUtf8(isolate, "_getWindowHeight"),
			v8::FunctionTemplate::New(isolate, __getWindowHeight));

#include "gluegen/glbindinit.h"

  return _gl->NewInstance();
}


#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_GlesJSLib_onSurfaceCreated
(JNIEnv *env, jclass clas) {
	LOGI("JNI onSurfaceCreated");

  scope([&]() {
    const int argc = 0;
    Local<Value> argv[argc] = {};
    callFunction("onSurfaceCreated", argc, argv);
  });
}

/* This does double duty as both the init and displaychanged function.
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_GlesJSLib_onSurfaceChanged
(JNIEnv *env, jclass clas, jint width, jint height) {
	LOGI("JNI onSurfaceChanged");

  scope([&]() {
    Isolate *isolate = service->GetIsolate();

    Handle<Number> js_width = v8::Integer::New(isolate, width);
    Handle<Number> js_height = v8::Integer::New(isolate, height);

    const int argc = 2;
    Local<Value> argv[argc] = {js_width, js_height};
    callFunction("onSurfaceChanged", argc, argv);
  });
}


JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_GlesJSLib_onNewFrame
(JNIEnv *env, jclass clas, jfloatArray headViewMatrix, jfloatArray headQuaternion) {
  jfloat* headViewMatrixElements = env->GetFloatArrayElements(headViewMatrix, 0);
  jfloat* headQuaternionElements = env->GetFloatArrayElements(headQuaternion, 0);

  Isolate::Scope isolate_scope(js->isolate);
	HandleScope handle_scope(js->isolate);

  const int argc = 2;
  Local<Float32Array> headMatrixFloat32Array = Float32Array::New(ArrayBuffer::New(js->isolate, 16 * 4), 0, 16);
  for (int i = 0; i < 16; i++) {
    headMatrixFloat32Array->Set(i, Number::New(js->isolate, headViewMatrixElements[i]));
  }
  Local<Float32Array> headQuaternionFloat32Array = Float32Array::New(ArrayBuffer::New(js->isolate, 4 * 4), 0, 4);
  for (int i = 0; i < 16; i++) {
    headQuaternionFloat32Array->Set(i, Number::New(js->isolate, headQuaternionElements[i]));
  }
	Local<Value> argv[argc] = {headMatrixFloat32Array, headQuaternionFloat32Array};
	callFunction("onDrawFrame", argc, argv);

  env->ReleaseFloatArrayElements(headViewMatrix, headViewMatrixElements, 0);
  env->ReleaseFloatArrayElements(headQuaternion, headQuaternionElements, 0);
}


JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_GlesJSLib_onDrawEye
(JNIEnv *env, jclass clasj, jfloatArray eyeViewMatrix, jfloatArray eyePerspectiveMatrix) {
  jfloat* eyeViewMatrixElements = env->GetFloatArrayElements(eyeViewMatrix, 0);
  jfloat* eyePerspectiveMatrixElements = env->GetFloatArrayElements(eyePerspectiveMatrix, 0);

  Isolate::Scope isolate_scope(js->isolate);
	HandleScope handle_scope(js->isolate);

  const int argc = 2;
  Local<Float32Array> eyeViewMatrixFloat32Array = Float32Array::New(ArrayBuffer::New(js->isolate, 16 * 4), 0, 16);
  for (int i = 0; i < 16; i++) {
    eyeViewMatrixFloat32Array->Set(i, Number::New(js->isolate, eyeViewMatrixElements[i]));
  }
  Local<Float32Array> eyePerspectiveMatrixFloat32Array = Float32Array::New(ArrayBuffer::New(js->isolate, 4 * 4), 0, 4);
  for (int i = 0; i < 16; i++) {
    eyePerspectiveMatrixFloat32Array->Set(i, Number::New(js->isolate, eyePerspectiveMatrixElements[i]));
  }
	Local<Value> argv[argc] = {eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array};
	callFunction("onDrawEye", argc, argv);

  env->ReleaseFloatArrayElements(eyeViewMatrix, eyeViewMatrixElements, 0);
  env->ReleaseFloatArrayElements(eyePerspectiveMatrix, eyePerspectiveMatrixElements, 0);
}


/*
 * Class:     com_mafintosh_nodeonandroid_GlesJSLib
 * Method:    onDrawFrame
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_GlesJSLib_onDrawFrame
(JNIEnv *env, jclass clas, jfloatArray viewMatrix, jfloatArray projectionMatrix) {
  jfloat* viewMatrixElements = env->GetFloatArrayElements(viewMatrix, 0);
  jfloat* projectionMatrixElements = env->GetFloatArrayElements(projectionMatrix, 0);

  Isolate::Scope isolate_scope(js->isolate);
	HandleScope handle_scope(js->isolate);

  const int argc = 2;
  Local<Float32Array> viewFloat32Array = Float32Array::New(ArrayBuffer::New(js->isolate, 16 * 4), 0, 16);
  for (int i = 0; i < 16; i++) {
    viewFloat32Array->Set(i, Number::New(js->isolate, viewMatrixElements[i]));
  }
  Local<Float32Array> projectionFloat32Array = Float32Array::New(ArrayBuffer::New(js->isolate, 16 * 4), 0, 16);
  for (int i = 0; i < 16; i++) {
    projectionFloat32Array->Set(i, Number::New(js->isolate, projectionMatrixElements[i]));
  }
	Local<Value> argv[argc] = {viewFloat32Array, projectionFloat32Array};
	callFunction("onDrawFrame", argc, argv);

  env->ReleaseFloatArrayElements(viewMatrix, viewMatrixElements, 0);
  env->ReleaseFloatArrayElements(projectionMatrix, projectionMatrixElements, 0);
}

/*
 * Class:     com_mafintosh_nodeonandroid_GlesJSLib
 * Method:    onTouchEvent
 * Signature: (DDZZZ)V
 */
// NOTE: must be called from the render thread. Multiple threads accessing
// isolate will cause crash.
JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_GlesJSLib_onTouchEvent
(JNIEnv *env, jclass clas, jint ptrid, jdouble x, jdouble y,
jboolean press, jboolean release) {
	//LOGI("JNI onTouchEvent");
	Isolate::Scope isolate_scope(js->isolate);
	HandleScope handle_scope(js->isolate);
	// jnienv = env;
	// pass new coords before passing up/down
	// pass coords to JS
	Handle<Value> js_ptrid = v8::Integer::New(js->isolate, ptrid);
	Handle<Value> js_x = v8::Integer::New(js->isolate, x);
	Handle<Value> js_y = v8::Integer::New(js->isolate, y);
	const int argc3 = 3;
	Local<Value> argv3[argc3] = { js_ptrid, js_x, js_y };
	callFunction("_mouseMoveCallback",argc3,argv3);
	if (press) {
		// start touch
		// pass event to JS
		const int argc1 = 1;
		Local<Value> argv1[argc1] = { js_ptrid };
		callFunction("_mouseDownCallback",argc1,argv1);
	}
	if (release) {
		// end touch
		// pass event to JS
		const int argc2 = 1;
		Local<Value> argv2[argc2] = { js_ptrid };
		callFunction("_mouseUpCallback",argc2,argv2);
	}
}


/*
 * Class:     com_mafintosh_nodeonandroid_GlesJSLib
 * Method:    onMultitouchCoordinates
 * Signature: (IDD)V
 */
JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_GlesJSLib_onMultitouchCoordinates
(JNIEnv * env, jclass clas, jint ptrid, jdouble x, jdouble y) {
	//LOGI("JNI MultitouchCoordinates");
	Isolate::Scope isolate_scope(js->isolate);
	HandleScope handle_scope(js->isolate);
	// jnienv = env;
	// pass coords to JS
	Handle<Value> js_ptrid = v8::Integer::New(js->isolate, ptrid);
	Handle<Value> js_x = v8::Integer::New(js->isolate, x);
	Handle<Value> js_y = v8::Integer::New(js->isolate, y);
	const int argc = 3;
	Local<Value> argv[argc] = { js_ptrid, js_x, js_y };
	callFunction("_touchCoordinatesCallback",argc,argv);
}


/*
 * Class:     com_mafintosh_nodeonandroid_GlesJSLib
 * Method:    onControllerEvent
 * Signature: (IZ[I[F)V
 */
JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_GlesJSLib_onControllerEvent
(JNIEnv *env, jclass clas, jint player, jboolean active,
jbooleanArray buttons, jfloatArray axes) {
	//LOGI("Controller event %d %d",player,active);
	// jnienv = env;
	// store active value
	gamepadvalues[PLAYERDATASIZE*player] = active ? 1 : 0;
	if (active) {
		// if active, store button and axes values
		jboolean* buttonsval = jnienv->GetBooleanArrayElements(buttons, 0);
		long buttonslen = jnienv->GetArrayLength(buttons);
		jfloat* axesval = jnienv->GetFloatArrayElements(axes, 0);
		long axeslen = jnienv->GetArrayLength(axes);
		//LOGI("######## %ld %ld",buttonslen,axeslen);
		for (int i=0; i<buttonslen; i++)
			gamepadvalues[PLAYERDATASIZE*player + 1 + i] = buttonsval[i];
		for (int i=0; i<axeslen; i++)
			gamepadvalues[PLAYERDATASIZE*player + 1+NR_BUTTONS+i] = axesval[i];

		jnienv->ReleaseBooleanArrayElements(buttons, buttonsval, JNI_ABORT);
		jnienv->ReleaseFloatArrayElements(axes, axesval, JNI_ABORT);
		//jnienv->DeleteLocalRef(buttons);
		//jnienv->DeleteLocalRef(axes);

	}
	// if not active, values are null, keep old values
}


/* JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_startNode__Ljava_lang_String_2Ljava_lang_String_2
(JNIEnv *env, jobject thiz, jstring jsPath, jstring port) {
  const char *nodeString = "node";
  const char *jsPathString = env->GetStringUTFChars(jsPath, NULL);
  const char *portString = env->GetStringUTFChars(port, NULL);
  char argsString[4096];
  int i = 0;

  char *nodeArg = argsString + i;
  strncpy(nodeArg, nodeString, sizeof(argsString) - i);
  i += strlen(nodeString) + 1;

  char *jsPathArg = argsString + i;
  strncpy(jsPathArg, jsPathString, sizeof(argsString) - i);
  i += strlen(jsPathString) + 1;

  char *portArg = argsString + i;
  strncpy(portArg, portString, sizeof(argsString) - i);
  i += strlen(portString) + 1;

  char *args[3] = {nodeArg, jsPathArg, portArg};
  node::Start(3, args);
} */


JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_start
(JNIEnv *env, jobject thiz, jstring jsPath, jstring port) {
  redirectStdioToLog();

  const char *nodeString = "node";
  const char *jsPathString = env->GetStringUTFChars(jsPath, NULL);
  const char *portString = env->GetStringUTFChars(port, NULL);
  char argsString[4096];
  int i = 0;

  char *nodeArg = argsString + i;
  strncpy(nodeArg, nodeString, sizeof(argsString) - i);
  i += strlen(nodeString) + 1;

  char *jsPathArg = argsString + i;
  strncpy(jsPathArg, jsPathString, sizeof(argsString) - i);
  i += strlen(jsPathString) + 1;

  char *portArg = argsString + i;
  strncpy(portArg, portString, sizeof(argsString) - i);
  i += strlen(portString) + 1;

  char *args[3] = {nodeArg, jsPathArg, portArg};
  // node::Start(3, args);
  // service = new node::NodeService(3, args);
  service = new node::NodeService(3, args);

  service->Scope([]() {
    service->GetContext()->Global()->Set(v8::String::NewFromUtf8(service->GetIsolate(), "gl"), makeGl());
  });
}

JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_tick
(JNIEnv *env, jobject thiz, jstring jsPath, jstring port) {
  service->Tick();
}


#ifdef __cplusplus
}
#endif


//END_INCLUDE(all)