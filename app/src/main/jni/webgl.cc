#include <cstring>
#include <vector>
#include <iostream>

#include "webgl.h"
#include "semaphore.h"
#include "util.h"
#include <node.h>
#include <node_buffer.h>
// #include <GL/glew.h>
// #include "GLES2/gl2platform.h"
// #include "GLES2/gl2ext.h"

#include <EGL/egl.h>
// at least some defs from gl1 are needed
#include <GLES/gl.h>
#include <GLES2/gl2.h>

#define CHECK_ARRAY_BUFFER(val) if(!val->IsArrayBufferView()) \
        {Nan::ThrowTypeError("Only support array buffer"); return;}

extern bool isUiThread;
extern void blockUiSoft(std::function<void()> fn);
extern void blockUiHard(std::function<void()> fn);

namespace webgl {

using namespace node;
using namespace v8;
using namespace std;

// forward declarations
/* enum GLObjectType {
  GLOBJECT_TYPE_BUFFER,
  GLOBJECT_TYPE_FRAMEBUFFER,
  GLOBJECT_TYPE_PROGRAM,
  GLOBJECT_TYPE_RENDERBUFFER,
  GLOBJECT_TYPE_SHADER,
  GLOBJECT_TYPE_TEXTURE,
};

void registerGLObj(GLObjectType type, GLuint obj);
void unregisterGLObj(GLuint obj); */

// A 32-bit and 64-bit compatible way of converting a pointer to a GLuint.
static GLuint ToGLuint(const void* ptr) {
  return static_cast<GLuint>(reinterpret_cast<size_t>(ptr));
}

template<typename Type>
inline Type* getArrayData(Local<Value> arg, int* num = NULL) {
  Type *data=NULL;
  if (num) *num=0;

  if (!arg->IsNull()) {
    if (arg->IsArray()) {
      Nan::ThrowError("Not support array type");
    } else if (arg->IsObject()) {
      Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(arg);
      if (num) *num=arr->ByteLength()/sizeof(Type);
      data = reinterpret_cast<Type*>((char *)arr->Buffer()->GetContents().Data() + arr->ByteOffset());
    } else {
      Nan::ThrowError("Bad array argument");
    }
  }

  return data;
}

inline void *getImageData(Local<Value> arg, int *num = NULL) {
  Isolate *isolate = Isolate::GetCurrent();

  void *pixels = NULL;
  if (!arg->IsNull()) {
    Local<Object> obj = Local<Object>::Cast(arg);
    if (obj->IsObject()) {
      if (obj->IsArrayBufferView()) {
        pixels = getArrayData<BYTE>(obj, num);
      } else {
        Local<String> dataString = String::NewFromUtf8(isolate, "data", NewStringType::kInternalized).ToLocalChecked();
        Local<Value> data = obj->Get(dataString);
        if (data->BooleanValue()) {
          pixels = getArrayData<BYTE>(data, num);
        } else {
          Nan::ThrowError("Bad texture argument");
          // pixels = node::Buffer::Data(Nan::Get(obj, JS_STR("data")).ToLocalChecked());
        }
      }
    } else {
      Nan::ThrowError("Bad texture argument");
    }
  }
  return pixels;
}

/* NAN_METHOD(Init) {
  Nan::HandleScope scope;
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    // Problem: glewInit failed, something is seriously wrong.
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    info.GetReturnValue().Set(JS_INT(-1));
  }else{
    //fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    info.GetReturnValue().Set(JS_INT(0));  
  } 
  
} */

NAN_METHOD(Uniform1f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();

  if (isUiThread) {
    glUniform1f(location, x);
  } else {
    blockUiSoft([=]() {
      glUniform1f(location, x);
    });
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform2f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();

  if (isUiThread) {
    glUniform2f(location, x, y);
  } else {
    blockUiSoft([=]() {
      glUniform2f(location, x, y);
    });
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform3f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();
  float z = (float)info[3]->NumberValue();

  if (isUiThread) {
    glUniform3f(location, x, y, z);
  } else {
    blockUiSoft([=]() {
      glUniform3f(location, x, y, z);
    });
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform4f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();
  float z = (float)info[3]->NumberValue();
  float w = (float)info[4]->NumberValue();

  if (isUiThread) {
    glUniform3f(location, x, y, z);
  } else {
    blockUiSoft([=]() {
      glUniform4f(location, x, y, z, w);
    });
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform1i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int x = info[1]->Int32Value();

  if (isUiThread) {
    glUniform1i(location, x);
  } else {
    blockUiSoft([=]() {
      glUniform1i(location, x);
    });
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform2i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int x = info[1]->Int32Value();
  int y = info[2]->Int32Value();

  if (isUiThread) {
    glUniform2i(location, x, y);
  } else {
    blockUiSoft([=]() {
      glUniform2i(location, x, y);
    });
  }
  // o.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform3i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int x = info[1]->Int32Value();
  int y = info[2]->Int32Value();
  int z = info[3]->Int32Value();

  if (isUiThread) {
    glUniform3i(location, x, y, z);
  } else {
    blockUiSoft([=]() {
      glUniform3i(location, x, y, z);
    });
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform4i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int x = info[1]->Int32Value();
  int y = info[2]->Int32Value();
  int z = info[3]->Int32Value();
  int w = info[4]->Int32Value();

  if (isUiThread) {
    glUniform4i(location, x, y, z, w);
  } else {
    blockUiSoft([=]() {
      glUniform4i(location, x, y, z, w);
    });
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform1fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    GLfloat *ptr = getArrayData<GLfloat>(float32Array, &num);
    if (isUiThread) {
      glUniform1fv(location, num, ptr);
    } else {
      GLfloat *ptr2 = cloneData(ptr, num * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform1fv(location, num, ptr2);
        free(ptr2);
      });
    }
  } else {
    GLfloat *ptr = getArrayData<GLfloat>(info[1], &num);
    if (isUiThread) {
      glUniform1fv(location, num, ptr);
    } else {
      GLfloat *ptr2 = cloneData(ptr, num * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform1fv(location, num, ptr2);
        free(ptr2);
      });
    }
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform2fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    GLfloat *ptr = getArrayData<GLfloat>(float32Array, &num);
    num /= 2;
    if (isUiThread) {
      glUniform2fv(location, num, ptr);
    } else {
      GLfloat *ptr2 = cloneData(ptr, num * 2 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform2fv(location, num, ptr2);
        free(ptr2);
      });
    }
  } else {
    GLfloat *ptr = getArrayData<GLfloat>(info[1], &num);
    num /= 2;
    if (isUiThread) {
      glUniform2fv(location, num, ptr);
    } else {
      GLfloat *ptr2 = cloneData(ptr, num * 2 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform2fv(location, num, ptr2);
        free(ptr2);
      });
    }
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform3fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    GLfloat *ptr = getArrayData<GLfloat>(float32Array, &num);
    num /= 3;
    if (isUiThread) {
      glUniform3fv(location, num, ptr);
    } else {
      GLfloat *ptr2 = cloneData(ptr, num * 3 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform3fv(location, num, ptr2);
        free(ptr2);
      });
    }
  } else {
    GLfloat *ptr = getArrayData<GLfloat>(info[1], &num);
    num /= 3;
    if (isUiThread) {
      glUniform3fv(location, num, ptr);
    } else {
      GLfloat *ptr2 = cloneData(ptr, num * 3 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform3fv(location, num, ptr2);
        free(ptr2);
      });
    }
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform4fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    GLfloat *ptr=getArrayData<GLfloat>(float32Array, &num);
    num /= 4;
    if (isUiThread) {
      glUniform4fv(location, num, ptr);
    } else {
      GLfloat *ptr2 = cloneData(ptr, num * 4 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform4fv(location, num, ptr2);
        free(ptr2);
      });
    }
  } else {
    GLfloat *ptr=getArrayData<GLfloat>(info[1], &num);
    num /= 4;
    if (isUiThread) {
      glUniform4fv(location, num, ptr);
    } else {
      GLfloat *ptr2 = cloneData(ptr, num * 4 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform4fv(location, num, ptr2);
        free(ptr2);
      });
    }
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform1iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      int32Array->Set(i, array->Get(i));
    }
    GLint *ptr = getArrayData<GLint>(int32Array, &num);
    if (isUiThread) {
      glUniform1iv(location, num, ptr);
    } else {
      GLint *ptr2 = cloneData(ptr, num * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform1iv(location, num, ptr2);
        free(ptr2);
      });
    }
  } else {
    GLint *ptr = getArrayData<GLint>(info[1], &num);
    if (isUiThread) {
      glUniform1iv(location, num, ptr);
    } else {
      GLint *ptr2 = cloneData(ptr, num * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform1iv(location, num, ptr2);
        free(ptr2);
      });
    }
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform2iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      int32Array->Set(i, array->Get(i));
    }
    GLint *ptr = getArrayData<GLint>(int32Array, &num);
    num /= 2;
    if (isUiThread) {
      glUniform2iv(location, num, ptr);
    } else {
      GLint *ptr2 = cloneData(ptr, num * 2 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform2iv(location, num, ptr2);
        free(ptr2);
      });
    }
  } else {
    GLint *ptr = getArrayData<GLint>(info[1], &num);
    num /= 2;
    if (isUiThread) {
      glUniform2iv(location, num, ptr);
    } else {
      GLint *ptr2 = cloneData(ptr, num * 2 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform2iv(location, num, ptr2);
        free(ptr2);
      });
    }
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform3iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      int32Array->Set(i, array->Get(i));
    }
    GLint *ptr = getArrayData<GLint>(int32Array, &num);
    num /= 3;
    if (isUiThread) {
      glUniform3iv(location, num, ptr);
    } else {
      GLint *ptr2 = cloneData(ptr, num * 3 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform3iv(location, num, ptr2);
        free(ptr2);
      });
    }
  } else {
    GLint *ptr=getArrayData<GLint>(info[1], &num);
    num /= 3;
    if (isUiThread) {
      glUniform3iv(location, num, ptr);
    } else {
      GLint *ptr2 = cloneData(ptr, num * 3 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform3iv(location, num, ptr2);
        free(ptr2);
      });
    }
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform4iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      int32Array->Set(i, array->Get(i));
    }
    GLint *ptr = getArrayData<GLint>(int32Array, &num);
    num /= 4;
    if (isUiThread) {
      glUniform4iv(location, num, ptr);
    } else {
      GLint *ptr2 = cloneData(ptr, num * 4 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform4iv(location, num, ptr2);
        free(ptr2);
      });
    }
  } else {
    GLint *ptr = getArrayData<GLint>(info[1], &num);
    num /= 4;
    if (isUiThread) {
      glUniform4iv(location, num, ptr);
    } else {
      GLint *ptr2 = cloneData(ptr, num * 4 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniform4iv(location, num, ptr2);
        free(ptr2);
      });
    }
  }
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PixelStorei) {
  Nan::HandleScope scope;

  int pname = info[0]->Int32Value();
  int param = info[1]->Int32Value();

  if (isUiThread) {
    glPixelStorei(pname, param);
  } else {
    blockUiSoft([=]() {
      glPixelStorei(pname, param);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BindAttribLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  int index = info[1]->Int32Value();
  String::Utf8Value name(info[2]);

  if (isUiThread) {
    glBindAttribLocation(program, index, *name);
  } else {
    char *name2 = cloneData(*name, name.length() + 1);
    blockUiSoft([=]() {
      glBindAttribLocation(program, index, name2);
      free(name2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(GetError) {
  Nan::HandleScope scope;

  GLint error;
  if (isUiThread) {
    error = glGetError();
  } else {
    volatile GLint localError;
    blockUiHard([&]() {
      localError = glGetError();
    });
    error = localError;
  }
  info.GetReturnValue().Set(Nan::New<Integer>(error));
}


NAN_METHOD(DrawArrays) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value();
  int first = info[1]->Int32Value();
  int count = info[2]->Int32Value();

  if (isUiThread) {
    glDrawArrays(mode, first, count);
  } else {
    blockUiSoft([=]() {
      glDrawArrays(mode, first, count);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DrawArraysInstancedANGLE) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value();
  int first = info[1]->Int32Value();
  int count = info[2]->Int32Value();
  int primcount = info[3]->Int32Value();

  if (isUiThread) {
    glDrawArraysInstanced(mode, first, count, primcount);
  } else {
    blockUiSoft([=]() {
      glDrawArraysInstanced(mode, first, count, primcount);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(UniformMatrix2fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value();
  GLboolean transpose = info[1]->BooleanValue();

  GLfloat *data;
  GLsizei count;
  // GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (info[2]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[2]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &count);
  } else {
    data = getArrayData<GLfloat>(info[2], &count);
  }

  if (count < 4) {
    Nan::ThrowError("Not enough data for UniformMatrix2fv");
  } else {
    count /= 4;
    if (isUiThread) {
      glUniformMatrix2fv(location, count, transpose, data);
    } else {
      GLfloat *data2 = cloneData(data, count * 4 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniformMatrix2fv(location, count, transpose, data2);
        free(data2);
      });
    }

    // info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(UniformMatrix3fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value();
  GLboolean transpose = info[1]->BooleanValue();

  GLfloat *data;
  GLsizei count;
  // GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (info[2]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[2]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &count);
  } else {
    data = getArrayData<GLfloat>(info[2], &count);
  }

  if (count < 9) {
    Nan::ThrowError("Not enough data for UniformMatrix3fv");
  }else{
    count /= 9;
    if (isUiThread) {
      glUniformMatrix3fv(location, count, transpose, data);
    } else {
      GLfloat *data2 = cloneData(data, count * 9 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniformMatrix3fv(location, count, transpose, data2);
        free(data2);
      });
    }

    // info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(UniformMatrix4fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value();
  GLboolean transpose = info[1]->BooleanValue();

  GLfloat *data;
  GLsizei count;
  // GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (info[2]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[2]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &count);
  } else {
    data = getArrayData<GLfloat>(info[2], &count);
  }

  if (count < 16) {
    Nan::ThrowError("Not enough data for UniformMatrix4fv");
  } else {
    count /= 16;
    if (isUiThread) {
      glUniformMatrix4fv(location, count, transpose, data);
    } else {
      GLfloat *data2 = cloneData(data, count * 16 * sizeof(GLfloat));
      blockUiSoft([=]() {
        glUniformMatrix4fv(location, count, transpose, data2);
        free(data2);
      });
    }

    // info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(GenerateMipmap) {
  Nan::HandleScope scope;

  GLint target = info[0]->Int32Value();

  if (isUiThread) {
    glGenerateMipmap(target);
  } else {
    blockUiSoft([=]() {
      glGenerateMipmap(target);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetAttribLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  String::Utf8Value name(info[1]);
  GLint result;

  if (isUiThread) {
    result = glGetAttribLocation(program, *name);
  } else {
    volatile GLint localResult;
    char *name2 = cloneData(*name, name.length() + 1);
    blockUiHard([&]() {
      localResult = glGetAttribLocation(program, name2);
      free(name2);
    });
    result = localResult;
  }
  info.GetReturnValue().Set(Nan::New<Number>(result));
}


NAN_METHOD(DepthFunc) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glDepthFunc(arg);
  } else {
    blockUiSoft([=]() {
      glDepthFunc(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(Viewport) {
  Nan::HandleScope scope;

  int x = info[0]->Int32Value();
  int y = info[1]->Int32Value();
  int width = info[2]->Int32Value();
  int height = info[3]->Int32Value();

  if (isUiThread) {
    glViewport(x, y, width, height);
  } else {
    blockUiSoft([=]() {
      glViewport(x, y, width, height);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CreateShader) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  GLuint shader;
  /* #ifdef LOGGING
  cout<<"createShader "<<shader<<endl;
  #endif */
  // registerGLObj(GLOBJECT_TYPE_SHADER, shader);
  if (isUiThread) {
    shader = glCreateShader(arg);
  } else {
    volatile GLuint localShader;
    blockUiHard([&]() {
      localShader = glCreateShader(arg);
    });
    shader = localShader;
  }
  info.GetReturnValue().Set(Nan::New<Number>(shader));
}


NAN_METHOD(ShaderSource) {
  Nan::HandleScope scope;

  int id = info[0]->Int32Value();
  String::Utf8Value code(info[1]);
  GLint length = code.length();

  if (isUiThread) {
    const char* codes[] = {*code};
    const GLint lengths[] = {length};
    glShaderSource(id, 1, codes, lengths);
  } else {
    char *codePtr2 = cloneData(*code, length + 1);
    blockUiSoft([=]() {
      const char* codes[] = {codePtr2};
      const GLint lengths[] = {length};
      glShaderSource(id, 1, codes, lengths);
      free(codePtr2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(CompileShader) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glCompileShader(arg);
  } else {
    blockUiSoft([=]() {
      glCompileShader(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(FrontFace) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glFrontFace(arg);
  } else {
    blockUiSoft([=]() {
      glFrontFace(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(GetShaderParameter) {
  Nan::HandleScope scope;

  int shader = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int value;
  switch (pname) {
    case GL_DELETE_STATUS:
    case GL_COMPILE_STATUS:
      if (isUiThread) {
        glGetShaderiv(shader, pname, &value);
      } else {
        volatile int localValue;
        blockUiHard([&]() {
          int localValue2;
          glGetShaderiv(shader, pname, &localValue2);
          localValue = localValue2;
        });
        value = localValue;
      }
      info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value)));
      break;
    case GL_SHADER_TYPE:
      if (isUiThread) {
        glGetShaderiv(shader, pname, &value);
      } else {
        volatile int localValue;
        blockUiHard([&]() {
          int localValue2;
          glGetShaderiv(shader, pname, &localValue2);
          localValue = localValue2;
        });
        value = localValue;
      }
      info.GetReturnValue().Set(JS_FLOAT(static_cast<unsigned long>(value)));
      break;
    case GL_INFO_LOG_LENGTH:
    case GL_SHADER_SOURCE_LENGTH:
      if (isUiThread) {
        glGetShaderiv(shader, pname, &value);
      } else {
        volatile int localValue;
        blockUiHard([&]() {
          int localValue2;
          glGetShaderiv(shader, pname, &localValue2);
          localValue = localValue2;
        });
        value = localValue;
      }
      info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
      break;
    default:
      Nan::ThrowTypeError("GetShaderParameter: Invalid Enum");
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetShaderInfoLog) {
  Nan::HandleScope scope;

  int id = info[0]->Int32Value();
  char Error[1024];
  int Len;

  if (isUiThread) {
    glGetShaderInfoLog(id, sizeof(Error), &Len, Error);
  } else {
    volatile char localError[sizeof(Error) / sizeof(Error[0])];
    volatile int localLen;
    blockUiHard([&]() {
      char localError2[sizeof(Error) / sizeof(Error[0])];
      int localLen2;
      glGetShaderInfoLog(id, sizeof(Error), &localLen2, localError2);
      memcpy((void *)localError, localError2, sizeof(Error));
      localLen = localLen2;
    });
    memcpy(Error, (void *)localError, sizeof(Error));
    Len = localLen;
  }

  info.GetReturnValue().Set(JS_STR(Error, Len));
}


NAN_METHOD(CreateProgram) {
  Nan::HandleScope scope;

  GLuint program;
  /* #ifdef LOGGING
  cout<<"createProgram "<<program<<endl;
  #endif */
  if (isUiThread) {
    program = glCreateProgram();
  } else {
    volatile GLuint localProgram;
    blockUiHard([&]() {
      localProgram = glCreateProgram();
    });
    program = localProgram;
  }
  // registerGLObj(GLOBJECT_TYPE_PROGRAM, program);

  info.GetReturnValue().Set(Nan::New<Number>(program));
}


NAN_METHOD(AttachShader) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  int shader = info[1]->Int32Value();

  if (isUiThread) {
    glAttachShader(program, shader);
  } else {
    blockUiSoft([=]() {
      glAttachShader(program, shader);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(LinkProgram) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glLinkProgram(arg); 
  } else {
    blockUiSoft([=]() {
      glLinkProgram(arg); 
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(GetProgramParameter) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int value;

  switch (pname) {
    case GL_DELETE_STATUS:
    case GL_LINK_STATUS:
    case GL_VALIDATE_STATUS:
      if (isUiThread) {
        glGetProgramiv(program, pname, &value);
      } else {
        volatile int localValue;
        blockUiHard([&]() {
          int localValue2;
          glGetProgramiv(program, pname, &localValue2);
          localValue = localValue2;
        });
        value = localValue;
      }
      info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value)));
      break;
    case GL_ATTACHED_SHADERS:
    case GL_ACTIVE_ATTRIBUTES:
    case GL_ACTIVE_UNIFORMS:
      if (isUiThread) {
        glGetProgramiv(program, pname, &value);
      } else {
        volatile int localValue;
        blockUiHard([&]() {
          int localValue2;
          glGetProgramiv(program, pname, &localValue2);
          localValue = localValue2;
        });
        value = localValue;
      }
      info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
      break;
    default:
      Nan::ThrowTypeError("GetProgramParameter: Invalid Enum");
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(GetUniformLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  v8::String::Utf8Value name(info[1]);
  GLint location;

  if (isUiThread) {
    location = glGetUniformLocation(program, *name);
  } else {
    char *name2 = cloneData(*name, name.length() + 1);
    volatile GLint localLocation;
    blockUiHard([&]() {
      localLocation = glGetUniformLocation(program, name2);
      free(name2);
    });
    location = localLocation;
  }
  info.GetReturnValue().Set(JS_INT(location));
}


NAN_METHOD(ClearColor) {
  Nan::HandleScope scope;

  float red = (float)info[0]->NumberValue();
  float green = (float)info[1]->NumberValue();
  float blue = (float)info[2]->NumberValue();
  float alpha = (float)info[3]->NumberValue();

  if (isUiThread) {
    glClearColor(red, green, blue, alpha);
  } else {
    blockUiSoft([=]() {
      glClearColor(red, green, blue, alpha);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(ClearDepth) {
  Nan::HandleScope scope;

  float depth = (float)info[0]->NumberValue();

  if (isUiThread) {
    glClearDepthf(depth);
  } else {
    blockUiSoft([=]() {
      glClearDepthf(depth);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Disable) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glDisable(arg);
  } else {
    blockUiSoft([=]() {
      glDisable(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Enable) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glEnable(arg);
  } else {
    blockUiSoft([=]() {
      glEnable(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(CreateTexture) {
  Nan::HandleScope scope;

  GLuint texture;
  /* #ifdef LOGGING
  cout<<"createTexture "<<texture<<endl;
  #endif */
  // registerGLObj(GLOBJECT_TYPE_TEXTURE, texture);

  if (isUiThread) {
    glGenTextures(1, &texture);
  } else {
    volatile GLuint localTexture;
    blockUiHard([&]() {
      GLuint localTexture2;
      glGenTextures(1, &localTexture2);
      localTexture = localTexture2;
    });
    texture = localTexture;
  }
  info.GetReturnValue().Set(Nan::New<Number>(texture));
}


NAN_METHOD(BindTexture) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int texture = info[1]->IsNull() ? 0 : info[1]->Int32Value();

  if (isUiThread) {
    glBindTexture(target, texture);
  } else {
    blockUiSoft([=]() {
      glBindTexture(target, texture);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

/* char texPixels[4096 * 4096 * 4];
NAN_METHOD(FlipTextureData) {
  Nan::HandleScope scope;

  int num;
  char *pixels=(char*)getArrayData<BYTE>(info[0], &num);
  int width = info[1]->Int32Value();
  int height = info[2]->Int32Value();

  int elementSize = num / width / height;
  for (int y = 0; y < height; y++) {
    memcpy(&(texPixels[(height - 1 - y) * width * elementSize]), &pixels[y * width * elementSize], width * elementSize);
  }
  memcpy(pixels, texPixels, num);
} */

NAN_METHOD(TexImage2D) {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::HandleScope scope;

  Local<Value> target = info[0];
  Local<Value> level = info[1];
  Local<Value> internalformat = info[2];
  Local<Value> width = info[3];
  Local<Value> height = info[4];
  Local<Value> border = info[5];
  Local<Value> format = info[6];
  Local<Value> type = info[7];
  Local<Value> pixels = info[8];

  Local<String> numberString = String::NewFromUtf8(isolate, "number", NewStringType::kInternalized).ToLocalChecked();
  Local<String> objectString = String::NewFromUtf8(isolate, "object", NewStringType::kInternalized).ToLocalChecked();
  Local<String> widthString = String::NewFromUtf8(isolate, "width", NewStringType::kInternalized).ToLocalChecked();
  Local<String> heightString = String::NewFromUtf8(isolate, "height", NewStringType::kInternalized).ToLocalChecked();

  if (info.Length() == 6) {
    // width is now format, height is now type, and border is now pixels
    if (
      target->TypeOf(isolate)->StrictEquals(numberString) &&
      level->TypeOf(isolate)->StrictEquals(numberString) && internalformat->TypeOf(isolate)->StrictEquals(numberString) &&
      width->TypeOf(isolate)->StrictEquals(numberString) && height->TypeOf(isolate)->StrictEquals(numberString) &&
      (border->IsNull() || (
        border->TypeOf(isolate)->StrictEquals(objectString) && border->ToObject()->Get(widthString)->TypeOf(isolate)->StrictEquals(numberString) && border->ToObject()->Get(heightString)->TypeOf(isolate)->StrictEquals(numberString)
      ))
    ) {
      pixels=border;
      /* if (pixels) {
        pixels = _getImageData(pixels);
      } */
      type=height;
      format=width;
      width = border->BooleanValue() ? border->ToObject()->Get(widthString) : Number::New(isolate, 1).As<Value>();
      height = border->BooleanValue() ? border->ToObject()->Get(heightString) : Number::New(isolate, 1).As<Value>();
      // return _texImage2D(target, level, internalformat, width, height, 0, format, type, pixels);
    } else {
      Nan::ThrowError("Expected texImage2D(number target, number level, number internalformat, number format, number type, Image pixels)");
      return;
    }
  } else if (info.Length() == 9) {
    if (
      target->TypeOf(isolate)->StrictEquals(numberString) &&
      level->TypeOf(isolate)->StrictEquals(numberString) && internalformat->TypeOf(isolate)->StrictEquals(numberString) &&
      width->TypeOf(isolate)->StrictEquals(numberString) && height->TypeOf(isolate)->StrictEquals(numberString) &&
      format->TypeOf(isolate)->StrictEquals(numberString) && type->TypeOf(isolate)->StrictEquals(numberString) &&
      (pixels->IsNull() || pixels->TypeOf(isolate)->StrictEquals(objectString))
    ) {
      /* if (pixels) {
        pixels = _getImageData(pixels);
      } */
      // return _texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    } else {
      Nan::ThrowError("Expected texImage2D(number target, number level, number internalformat, number width, number height, number border, number format, number type, ArrayBufferView pixels)");
      return;
    }
  } else {
    Nan::ThrowError("Bad texture argument");
    return;
  }

  int targetV = target->Int32Value();
  int levelV = level->Int32Value();
  int internalformatV = internalformat->Int32Value();
  int widthV = width->Int32Value();
  int heightV = height->Int32Value();
  int borderV = border->Int32Value();
  int formatV = format->Int32Value();
  int typeV = type->Int32Value();
  int num;
  char *pixelsV = (char *)getImageData(pixels, &num);

  if (isUiThread) {
    glTexImage2D(targetV, levelV, internalformatV, widthV, heightV, borderV, formatV, typeV, pixelsV);
  } else {
    char *pixelsV2 = cloneData(pixelsV, num);
    blockUiSoft([=]() {
      glTexImage2D(targetV, levelV, internalformatV, widthV, heightV, borderV, formatV, typeV, pixelsV2);
      free(pixelsV2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(TexParameteri) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int param = info[2]->Int32Value();

  if (isUiThread) {
    glTexParameteri(target, pname, param);
  } else {
    blockUiSoft([=]() {
      glTexParameteri(target, pname, param);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(TexParameterf) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  float param = (float) info[2]->NumberValue();

  if (isUiThread) {
    glTexParameterf(target, pname, param);
  } else {
    blockUiSoft([=]() {
      glTexParameterf(target, pname, param);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(Clear) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glClear(arg);
  } else {
    blockUiSoft([=]() {
      glClear(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(UseProgram) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glUseProgram(arg);
  } else {
    blockUiSoft([=]() {
      glUseProgram(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CreateBuffer) {
  Nan::HandleScope scope;

  GLuint buffer;
  /* #ifdef LOGGING
  cout<<"createBuffer "<<buffer<<endl;
  #endif */
  // registerGLObj(GLOBJECT_TYPE_BUFFER, buffer);

  if (isUiThread) {
    glGenBuffers(1, &buffer);
  } else {
    volatile GLuint localBuffer;
    blockUiHard([&]() {
      GLuint localBuffer2;
      glGenBuffers(1, &localBuffer2);
      localBuffer = localBuffer2;
    });
    buffer = localBuffer;
  }
  info.GetReturnValue().Set(Nan::New<Number>(buffer));
}

NAN_METHOD(BindBuffer) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int buffer = info[1]->Uint32Value();

  if (isUiThread) {
    glBindBuffer(target, buffer);
  } else {
    blockUiSoft([=]() {
      glBindBuffer(target, buffer);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(CreateFramebuffer) {
  Nan::HandleScope scope;

  GLuint buffer;
  /* #ifdef LOGGING
  cout<<"createFrameBuffer "<<buffer<<endl;
  #endif */
  // registerGLObj(GLOBJECT_TYPE_FRAMEBUFFER, buffer);

  if (isUiThread) {
    glGenFramebuffers(1, &buffer);
  } else {
    volatile GLuint localBuffer;
    blockUiHard([&]() {
      GLuint localBuffer2;
      glGenFramebuffers(1, &localBuffer2);
      localBuffer = localBuffer2;
    });
    buffer = localBuffer;
  }
  info.GetReturnValue().Set(Nan::New<Number>(buffer));
}


NAN_METHOD(BindFramebuffer) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int buffer = info[1]->IsNull() ? 0 : info[1]->Int32Value();

  if (isUiThread) {
    glBindFramebuffer(target, buffer);
  } else {
    blockUiSoft([=]() {
      glBindFramebuffer(target, buffer);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(FramebufferTexture2D) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int attachment = info[1]->Int32Value();
  int textarget = info[2]->Int32Value();
  int texture = info[3]->Int32Value();
  int level = info[4]->Int32Value();

  if (isUiThread) {
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
  } else {
    blockUiSoft([=]() {
      glFramebufferTexture2D(target, attachment, textarget, texture, level);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BufferData) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  if(info[1]->IsObject()) {
    Local<Object> obj = Local<Object>::Cast(info[1]);
    GLenum usage = info[2]->Int32Value();

    CHECK_ARRAY_BUFFER(obj);

    int element_size = 1;
    Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
    int size = arr->ByteLength() * element_size;
    char *data = (char *)arr->Buffer()->GetContents().Data() + arr->ByteOffset();

    if (isUiThread) {
      glBufferData(target, size, data, usage);
    } else {
      char *data2 = cloneData(data, size);
      blockUiSoft([=]() {
        glBufferData(target, size, data2, usage);
        free(data2);
      });
    }
  } else if(info[1]->IsNumber()) {
    GLsizeiptr size = info[1]->Uint32Value();
    GLenum usage = info[2]->Int32Value();
    if (isUiThread) {
      glBufferData(target, size, NULL, usage);
    } else {
      blockUiSoft([=]() {
        glBufferData(target, size, NULL, usage);
      });
    }
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(BufferSubData) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int offset = info[1]->Int32Value();
  Local<Object> obj = Local<Object>::Cast(info[2]);

  int element_size = 1;
  Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
  int size = arr->ByteLength() * element_size;
  char *data = (char *)arr->Buffer()->GetContents().Data() + arr->ByteOffset();

  if (isUiThread) {
    glBufferSubData(target, offset, size, data);
  } else {
    char *data2 = cloneData(data, size);
    blockUiSoft([=]() {
      glBufferSubData(target, offset, size, data2);
      free(data2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(BlendEquation) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value();

  if (isUiThread) {
    glBlendEquation(mode);
  } else {
    blockUiSoft([=]() {
      glBlendEquation(mode);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(BlendFunc) {
  Nan::HandleScope scope;

  int sfactor = info[0]->Int32Value();;
  int dfactor = info[1]->Int32Value();;

  if (isUiThread) {
    glBlendFunc(sfactor, dfactor);
  } else {
    blockUiSoft([=]() {
      glBlendFunc(sfactor, dfactor);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(EnableVertexAttribArray) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glEnableVertexAttribArray(arg);
  } else {
    blockUiSoft([=]() {
      glEnableVertexAttribArray(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(VertexAttribPointer) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  int size = info[1]->Int32Value();
  int type = info[2]->Int32Value();
  int normalized = info[3]->BooleanValue();
  int stride = info[4]->Int32Value();
  long offset = info[5]->Int32Value();

  //    printf("VertexAttribPointer %d %d %d %d %d %d\n", indx, size, type, normalized, stride, offset);
  if (isUiThread) {
    glVertexAttribPointer(indx, size, type, normalized, stride, (const GLvoid *)offset);
  } else {
    blockUiSoft([=]() {
      glVertexAttribPointer(indx, size, type, normalized, stride, (const GLvoid *)offset);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(ActiveTexture) {
  Nan::HandleScope scope;

  int arg = info[0]->Int32Value();
  if (isUiThread) {
    glActiveTexture(arg);
  } else {
    blockUiSoft([=]() {
      glActiveTexture(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(DrawElements) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value();
  int count = info[1]->Int32Value();
  int type = info[2]->Int32Value();
  GLvoid *offset = reinterpret_cast<GLvoid*>(info[3]->Uint32Value());

  if (isUiThread) {
    glDrawElements(mode, count, type, offset);
  } else {
    blockUiSoft([=]() {
      glDrawElements(mode, count, type, offset);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DrawElementsInstancedANGLE) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value();
  int count = info[1]->Int32Value();
  int type = info[2]->Int32Value();
  GLvoid *offset = reinterpret_cast<GLvoid*>(info[3]->Uint32Value());
  int primcount = info[4]->Int32Value();

  if (isUiThread) {
    glDrawElementsInstanced(mode, count, type, offset, primcount);
  } else {
    blockUiSoft([=]() {
      glDrawElementsInstanced(mode, count, type, offset, primcount);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(Flush) {
  // Nan::HandleScope scope;

  if (isUiThread) {
    glFlush();
  } else {
    blockUiSoft([=]() {
      glFlush();
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Finish) {
  // Nan::HandleScope scope;

  if (isUiThread) {
    glFinish();
  } else {
    blockUiSoft([=]() {
      glFinish();
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib1f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();

  if (isUiThread) {
    glVertexAttrib1f(indx, x);
  } else {
    blockUiSoft([=]() {
      glVertexAttrib1f(indx, x);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib2f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();

  if (isUiThread) {
    glVertexAttrib2f(indx, x, y);
  } else {
    blockUiSoft([=]() {
      glVertexAttrib2f(indx, x, y);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib3f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();
  float z = (float)info[3]->NumberValue();

  if (isUiThread) {
    glVertexAttrib3f(indx, x, y, z);
  } else {
    blockUiSoft([=]() {
      glVertexAttrib3f(indx, x, y, z);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib4f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();
  float z = (float)info[3]->NumberValue();
  float w = (float)info[4]->NumberValue();

  if (isUiThread) {
    glVertexAttrib4f(indx, x, y, z, w);
  } else {
    blockUiSoft([=]() {
      glVertexAttrib4f(indx, x, y, z, w);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib1fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  GLfloat *data;
  int num;
  // GLfloat *data = getArrayData<GLfloat>(info[1]);

  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &num);
  } else {
    data = getArrayData<GLfloat>(info[1], &num);
  }

  if (isUiThread) {
    glVertexAttrib1fv(indx, data);
  } else {
    GLfloat *data2 = cloneData(data, num * sizeof(GLfloat));
    blockUiSoft([=]() {
      glVertexAttrib1fv(indx, data2);
      free(data2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib2fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  GLfloat *data;
  int num;
  // GLfloat *data = getArrayData<GLfloat>(info[1]);

  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data=getArrayData<GLfloat>(float32Array, &num);
  } else {
    data=getArrayData<GLfloat>(info[1], &num);
  }

  if (isUiThread) {
    glVertexAttrib2fv(indx, data);
  } else {
    GLfloat *data2 = cloneData(data, num * sizeof(GLfloat));
    blockUiSoft([=]() {
      glVertexAttrib2fv(indx, data2);
      free(data2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib3fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  GLfloat *data;
  int num;
  // GLfloat *data = getArrayData<GLfloat>(info[1]);

  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &num);
  } else {
    data = getArrayData<GLfloat>(info[1], &num);
  }

  if (isUiThread) {
    glVertexAttrib3fv(indx, data);
  } else {
    GLfloat *data2 = cloneData(data, num * sizeof(GLfloat));
    blockUiSoft([=]() {
      glVertexAttrib3fv(indx, data2);
      free(data2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib4fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  GLfloat *data;
  int num;
  // GLfloat *data = getArrayData<GLfloat>(info[1]);

  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data=getArrayData<GLfloat>(float32Array, &num);
  } else {
    data=getArrayData<GLfloat>(info[1], &num);
  }

  if (isUiThread) {
    glVertexAttrib4fv(indx, data);
  } else {
    GLfloat *data2 = cloneData(data, num * sizeof(GLfloat));
    blockUiSoft([=]() {
      glVertexAttrib4fv(indx, data2);
      free(data2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttribDivisorANGLE) {
  Nan::HandleScope scope;

  unsigned int index = info[0]->Uint32Value();
  unsigned int divisor = info[1]->Uint32Value();

  if (isUiThread) {
    glVertexAttribDivisor(index, divisor);
  } else {
    blockUiSoft([=]() {
      glVertexAttribDivisor(index, divisor);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BlendColor) {
  Nan::HandleScope scope;

  GLclampf r = (float)info[0]->NumberValue();
  GLclampf g = (float)info[1]->NumberValue();
  GLclampf b = (float)info[2]->NumberValue();
  GLclampf a = (float)info[3]->NumberValue();

  if (isUiThread) {
    glBlendColor(r,g,b,a);
  } else {
    blockUiSoft([=]() {
      glBlendColor(r, g, b, a);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BlendEquationSeparate) {
  Nan::HandleScope scope;

  GLenum modeRGB = info[0]->Int32Value();
  GLenum modeAlpha = info[1]->Int32Value();

  if (isUiThread) {
    glBlendEquationSeparate(modeRGB, modeAlpha);
  } else {
    blockUiSoft([=]() {
      glBlendEquationSeparate(modeRGB, modeAlpha);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BlendFuncSeparate) {
  Nan::HandleScope scope;

  GLenum srcRGB = info[0]->Int32Value();
  GLenum dstRGB = info[1]->Int32Value();
  GLenum srcAlpha = info[2]->Int32Value();
  GLenum dstAlpha = info[3]->Int32Value();

  if (isUiThread) {
    glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
  } else {
    blockUiSoft([=]() {
      glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(ClearStencil) {
  Nan::HandleScope scope;

  GLint s = info[0]->Int32Value();

  if (isUiThread) {
    glClearStencil(s);
  } else {
    blockUiSoft([=]() {
      glClearStencil(s);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(ColorMask) {
  Nan::HandleScope scope;

  GLboolean r = info[0]->BooleanValue();
  GLboolean g = info[1]->BooleanValue();
  GLboolean b = info[2]->BooleanValue();
  GLboolean a = info[3]->BooleanValue();

  if (isUiThread) {
    glColorMask(r, g, b, a);
  } else {
    blockUiSoft([=]() {
      glColorMask(r, g, b, a);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CopyTexImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLint level = info[1]->Int32Value();
  GLenum internalformat = info[2]->Int32Value();
  GLint x = info[3]->Int32Value();
  GLint y = info[4]->Int32Value();
  GLsizei width = info[5]->Int32Value();
  GLsizei height = info[6]->Int32Value();
  GLint border = info[7]->Int32Value();

  if (isUiThread) {
    glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
  } else {
    blockUiSoft([=]() {
      glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CopyTexSubImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLint level = info[1]->Int32Value();
  GLint xoffset = info[2]->Int32Value();
  GLint yoffset = info[3]->Int32Value();
  GLint x = info[4]->Int32Value();
  GLint y = info[5]->Int32Value();
  GLsizei width = info[6]->Int32Value();
  GLsizei height = info[7]->Int32Value();

  if (isUiThread) {
    glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
  } else {
    blockUiSoft([=]() {
      glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CullFace) {
  Nan::HandleScope scope;

  GLenum mode = info[0]->Int32Value();

  if (isUiThread) {
    glCullFace(mode);
  } else {
    blockUiSoft([=]() {
      glCullFace(mode);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DepthMask) {
  Nan::HandleScope scope;

  GLboolean flag = info[0]->BooleanValue();

  if (isUiThread) {
    glDepthMask(flag);
  } else {
    blockUiSoft([=]() {
      glDepthMask(flag);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DepthRange) {
  Nan::HandleScope scope;

  GLclampf zNear = (float) info[0]->NumberValue();
  GLclampf zFar = (float) info[1]->NumberValue();

  if (isUiThread) {
    glDepthRangef(zNear, zFar);
  } else {
    blockUiSoft([=]() {
      glDepthRangef(zNear, zFar);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DisableVertexAttribArray) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Int32Value();

  if (isUiThread) {
    glDisableVertexAttribArray(index);
  } else {
    blockUiSoft([=]() {
      glDisableVertexAttribArray(index);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Hint) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum mode = info[1]->Int32Value();

  if (isUiThread) {
    glHint(target, mode);
  } else {
    blockUiSoft([=]() {
      glHint(target, mode);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(IsEnabled) {
  Nan::HandleScope scope;

  GLenum cap = info[0]->Int32Value();
  bool ret;

  if (isUiThread) {
    ret = glIsEnabled(cap) != 0;
  } else {
    volatile bool localRet;
    blockUiHard([&]() {
      localRet = glIsEnabled(cap) != 0;
    });
    ret = localRet;
  }
  info.GetReturnValue().Set(Nan::New<Boolean>(ret));
}

NAN_METHOD(LineWidth) {
  Nan::HandleScope scope;

  GLfloat width = (float) info[0]->NumberValue();

  if (isUiThread) {
    glLineWidth(width);
  } else {
    blockUiSoft([=]() {
      glLineWidth(width);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PolygonOffset) {
  Nan::HandleScope scope;

  GLfloat factor = (float) info[0]->NumberValue();
  GLfloat units = (float) info[1]->NumberValue();

  if (isUiThread) {
    glPolygonOffset(factor, units);
  } else {
    blockUiSoft([=]() {
      glPolygonOffset(factor, units);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(SampleCoverage) {
  Nan::HandleScope scope;

  GLclampf value = (float) info[0]->NumberValue();
  GLboolean invert = info[1]->BooleanValue();

  if (isUiThread) {
    glSampleCoverage(value, invert);
  } else {
    blockUiSoft([=]() {
      glSampleCoverage(value, invert);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Scissor) {
  Nan::HandleScope scope;

  GLint x = info[0]->Int32Value();
  GLint y = info[1]->Int32Value();
  GLsizei width = info[2]->Int32Value();
  GLsizei height = info[3]->Int32Value();

  if (isUiThread) {
    glScissor(x, y, width, height);
  } else {
    blockUiSoft([=]() {
      glScissor(x, y, width, height);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilFunc) {
  Nan::HandleScope scope;

  GLenum func = info[0]->Int32Value();
  GLint ref = info[1]->Int32Value();
  GLuint mask = info[2]->Int32Value();

  if (isUiThread) {
    glStencilFunc(func, ref, mask);
  } else {
    blockUiSoft([=]() {
      glStencilFunc(func, ref, mask);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilFuncSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value();
  GLenum func = info[1]->Int32Value();
  GLint ref = info[2]->Int32Value();
  GLuint mask = info[3]->Int32Value();

  if (isUiThread) {
    glStencilFuncSeparate(face, func, ref, mask);
  } else {
    blockUiSoft([=]() {
      glStencilFuncSeparate(face, func, ref, mask);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilMask) {
  Nan::HandleScope scope;

  GLuint mask = info[0]->Uint32Value();

  if (isUiThread) {
    glStencilMask(mask);
  } else {
    blockUiSoft([=]() {
      glStencilMask(mask);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilMaskSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value();
  GLuint mask = info[1]->Uint32Value();

  if (isUiThread) {
    glStencilMaskSeparate(face, mask);
  } else {
    blockUiSoft([=]() {
      glStencilMaskSeparate(face, mask);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilOp) {
  Nan::HandleScope scope;

  GLenum fail = info[0]->Int32Value();
  GLenum zfail = info[1]->Int32Value();
  GLenum zpass = info[2]->Int32Value();

  if (isUiThread) {
    glStencilOp(fail, zfail, zpass);
  } else {
    blockUiSoft([=]() {
      glStencilOp(fail, zfail, zpass);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilOpSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value();
  GLenum fail = info[1]->Int32Value();
  GLenum zfail = info[2]->Int32Value();
  GLenum zpass = info[3]->Int32Value();

  if (isUiThread) {
    glStencilOpSeparate(face, fail, zfail, zpass);
  } else {
    blockUiSoft([=]() {
      glStencilOpSeparate(face, fail, zfail, zpass);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BindRenderbuffer) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLuint buffer = info[1]->IsNull() ? 0 : info[1]->Int32Value();

  if (isUiThread) {
    glBindRenderbuffer(target, buffer);
  } else {
    blockUiSoft([=]() {
      glBindRenderbuffer(target, buffer);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CreateRenderbuffer) {
  Nan::HandleScope scope;

  GLuint renderbuffers;
  /* #ifdef LOGGING
  cout<<"createRenderBuffer "<<renderbuffers<<endl;
  #endif */
  if (isUiThread) {
    glGenRenderbuffers(1, &renderbuffers);
  } else {
    volatile GLuint localRenderbuffers;
    blockUiHard([&]() {
      GLuint localRenderbuffers2;
      glGenRenderbuffers(1, &localRenderbuffers2);
      localRenderbuffers = localRenderbuffers2;
    });
    renderbuffers = localRenderbuffers;
  }
  // registerGLObj(GLOBJECT_TYPE_RENDERBUFFER, renderbuffers);

  info.GetReturnValue().Set(Nan::New<Number>(renderbuffers));
}

NAN_METHOD(DeleteBuffer) {
  Nan::HandleScope scope;

  GLuint buffer = info[0]->Uint32Value();

  if (isUiThread) {
    glDeleteBuffers(1, &buffer);
  } else {
    blockUiSoft([=]() {
      glDeleteBuffers(1, &buffer);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteFramebuffer) {
  Nan::HandleScope scope;

  GLuint buffer = info[0]->Uint32Value();

  if (isUiThread) {
    glDeleteFramebuffers(1, &buffer);
  } else {
    blockUiSoft([=]() {
      glDeleteFramebuffers(1, &buffer);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteProgram) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Uint32Value();

  if (isUiThread) {
    glDeleteProgram(program);
  } else {
    blockUiSoft([=]() {
      glDeleteProgram(program);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteRenderbuffer) {
  Nan::HandleScope scope;

  GLuint renderbuffer = info[0]->Uint32Value();

  if (isUiThread) {
    glDeleteRenderbuffers(1, &renderbuffer);
  } else {
    blockUiSoft([=]() {
      glDeleteRenderbuffers(1, &renderbuffer);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteShader) {
  Nan::HandleScope scope;

  GLuint shader = info[0]->Uint32Value();

  if (isUiThread) {
    glDeleteShader(shader);
  } else {
    blockUiSoft([=]() {
      glDeleteShader(shader);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteTexture) {
  Nan::HandleScope scope;

  GLuint texture = info[0]->Uint32Value();

  if (isUiThread) {
    glDeleteTextures(1, &texture);
  } else {
    blockUiSoft([=]() {
      glDeleteTextures(1, &texture);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DetachShader) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Uint32Value();
  GLuint shader = info[1]->Uint32Value();

  if (isUiThread) {
    glDetachShader(program, shader);
  } else {
    blockUiSoft([=]() {
      glDetachShader(program, shader);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(FramebufferRenderbuffer) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum attachment = info[1]->Int32Value();
  GLenum renderbuffertarget = info[2]->Int32Value();
  GLuint renderbuffer = info[3]->Uint32Value();

  if (isUiThread) {
    glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
  } else {
    blockUiSoft([=]() {
      glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetVertexAttribOffset) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Uint32Value();
  GLenum pname = info[1]->Int32Value();
  void *ret = nullptr;

  if (isUiThread) {
    glGetVertexAttribPointerv(index, pname, &ret);
  } else {
    void * volatile localRet;
    blockUiHard([&]() {
      void *localRet2;
      glGetVertexAttribPointerv(index, pname, &localRet2);
      localRet = localRet2;
    });
    ret = localRet;
  }

  info.GetReturnValue().Set(JS_INT(ToGLuint(ret)));
}

NAN_METHOD(IsBuffer) {
  Nan::HandleScope scope;

  GLuint arg = info[0]->Uint32Value();
  bool ret;

  if (isUiThread) {
    ret = glIsBuffer(arg) != 0;
  } else {
    volatile bool localRet;
    blockUiHard([&]() {
      localRet = glIsBuffer(arg) != 0;
    });
    ret = localRet;
  }
  info.GetReturnValue().Set(Nan::New<Boolean>(ret));
}

NAN_METHOD(IsFramebuffer) {
  Nan::HandleScope scope;

  GLuint arg = info[0]->Uint32Value();
  bool ret;

  if (isUiThread) {
    ret = glIsFramebuffer(arg) != 0;
  } else {
    volatile bool localRet;
    blockUiHard([&]() {
      localRet = glIsFramebuffer(arg) != 0;
    });
    ret = localRet;
  }

  info.GetReturnValue().Set(JS_BOOL(ret));
}

NAN_METHOD(IsProgram) {
  Nan::HandleScope scope;

  GLuint arg = info[0]->Uint32Value();
  bool ret;

  if (isUiThread) {
    ret = glIsProgram(arg) != 0;
  } else {
    volatile bool localRet;
    blockUiHard([&]() {
      localRet = glIsProgram(arg) != 0;
    });
    ret = localRet;
  }

  info.GetReturnValue().Set(JS_BOOL(ret));
}

NAN_METHOD(IsRenderbuffer) {
  Nan::HandleScope scope;

  GLuint arg = info[0]->Uint32Value();
  bool ret;

  if (isUiThread) {
    ret = glIsRenderbuffer(arg) != 0;
  } else {
    volatile bool localRet;
    blockUiHard([&]() {
      localRet = glIsRenderbuffer(arg) != 0;
    });
    ret = localRet;
  }

  info.GetReturnValue().Set(JS_BOOL(ret));
}

NAN_METHOD(IsShader) {
  Nan::HandleScope scope;

  GLuint arg = info[0]->Uint32Value();
  bool ret;

  if (isUiThread) {
    ret = glIsShader(arg) != 0;
  } else {
    volatile bool localRet;
    blockUiHard([&]() {
      localRet = glIsShader(arg) != 0;
    });
    ret = localRet;
  }

  info.GetReturnValue().Set(JS_BOOL(ret));
}

NAN_METHOD(IsTexture) {
  Nan::HandleScope scope;

  GLuint arg = info[0]->Uint32Value();
  bool ret;

  if (isUiThread) {
    ret = glIsTexture(arg) != 0;
  } else {
    volatile bool localRet;
    blockUiHard([&]() {
      localRet = glIsTexture(arg) != 0;
    });
    ret = localRet;
  }

  info.GetReturnValue().Set(JS_BOOL(ret));
}

NAN_METHOD(RenderbufferStorage) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum internalformat = info[1]->Int32Value();
  GLsizei width = info[2]->Uint32Value();
  GLsizei height = info[3]->Uint32Value();

  if (isUiThread) {
    glRenderbufferStorage(target, internalformat, width, height);
  } else {
    blockUiSoft([=]() {
      glRenderbufferStorage(target, internalformat, width, height);
    });
  }
  
  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetShaderSource) {
  Nan::HandleScope scope;

  int shader = info[0]->Int32Value();

  GLint len;
  glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &len);
  GLchar *source = new GLchar[len];

  if (isUiThread) {
    glGetShaderSource(shader, len, nullptr, source);
  } else {
    blockUiHard([&]() {
      glGetShaderSource(shader, len, nullptr, source);
    });
  }

  Local<String> str = JS_STR(source, len);
  delete[] source;

  info.GetReturnValue().Set(str);
}

NAN_METHOD(ValidateProgram) {
  Nan::HandleScope scope;

  GLint arg = info[0]->Int32Value();
  if (isUiThread) {
    glValidateProgram(arg);
  } else {
    blockUiSoft([=]() {
      glValidateProgram(arg);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(TexSubImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLint level = info[1]->Int32Value();
  GLint xoffset = info[2]->Int32Value();
  GLint yoffset = info[3]->Int32Value();
  GLsizei width = info[4]->Int32Value();
  GLsizei height = info[5]->Int32Value();
  GLenum format = info[6]->Int32Value();
  GLenum type = info[7]->Int32Value();
  int num;
  char *pixels = (char*)getImageData(info[8], &num);

  /* if (pixels != nullptr) {
    int elementSize = num / width / height;
    for (int y = 0; y < height; y++) {
      memcpy(&(texPixels[(height - 1 - y) * width * elementSize]), &pixels[y * width * elementSize], width * elementSize);
    }
  } */
  if (isUiThread) {
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
  } else {
    char *pixels2 = cloneData(pixels, num);
    blockUiSoft([=]() {
      glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels2);
      free(pixels2);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(ReadPixels) {
  Nan::HandleScope scope;

  GLint x = info[0]->Int32Value();
  GLint y = info[1]->Int32Value();
  GLsizei width = info[2]->Int32Value();
  GLsizei height = info[3]->Int32Value();
  GLenum format = info[4]->Int32Value();
  GLenum type = info[5]->Int32Value();
  char *pixels = (char *)getImageData(info[6]);

  if (isUiThread) {
    glReadPixels(x, y, width, height, format, type, pixels);
  } else {
    blockUiHard([&]() {
      glReadPixels(x, y, width, height, format, type, pixels);
    });
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetTexParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum pname = info[1]->Int32Value();
  GLint value;

  if (isUiThread) {
    glGetTexParameteriv(target, pname, &value);
  } else {
    volatile GLint localValue;
    blockUiHard([&]() {
      GLint localValue2;
      glGetTexParameteriv(target, pname, &localValue2);
      localValue = localValue2;
    });
    value = localValue;
  }

  info.GetReturnValue().Set(Nan::New<Number>(value));
}

NAN_METHOD(GetActiveAttrib) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();
  GLuint index = info[1]->Int32Value();

  char name[1024];
  GLsizei length;
  GLsizei size;
  GLenum type;

  if (isUiThread) {
    glGetActiveAttrib(program, index, sizeof(name), &length, &size, &type, name);
  } else {
    volatile GLsizei localLength;
    volatile GLsizei localSize;
    volatile GLenum localType;
    volatile char localName[sizeof(name) / sizeof(name[0])];
    blockUiHard([&]() {
      GLsizei localLength2;
      GLsizei localSize2;
      GLenum localType2;
      char localName2[sizeof(name) / sizeof(name[0])];
      glGetActiveAttrib(program, index, sizeof(name), &localLength2, &localSize2, &localType2, localName2);
      localLength = localLength2;
      localSize = localSize2;
      localType = localType2;
      memcpy((void *)localName, localName2, sizeof(name));
    });
    length = localLength;
    size = localSize;
    type = localType;
    memcpy(name, (void *)localName, sizeof(name));
  }

  Local<Object> activeInfo = Nan::New<Object>();
  activeInfo->Set(JS_STR("size"), JS_INT(size));
  activeInfo->Set(JS_STR("type"), JS_INT((int)type));
  activeInfo->Set(JS_STR("name"), JS_STR(name, length));

  info.GetReturnValue().Set(activeInfo);
}

NAN_METHOD(GetActiveUniform) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();
  GLuint index = info[1]->Int32Value();

  char name[1024];
  GLsizei length;
  GLsizei size;
  GLenum type;
  if (isUiThread) {
    glGetActiveUniform(program, index, sizeof(name), &length, &size, &type, name);
  } else {
    volatile GLsizei localLength;
    volatile GLsizei localSize;
    volatile GLenum localType;
    volatile char localName[sizeof(name) / sizeof(name[0])];
    blockUiHard([&]() {
      GLsizei localLength2;
      GLsizei localSize2;
      GLenum localType2;
      char localName2[sizeof(name) / sizeof(name[0])];
      glGetActiveUniform(program, index, sizeof(name), &localLength2, &localSize2, &localType2, localName2);
      localLength = localLength2;
      localSize = localSize2;
      localType = localType2;
      memcpy((void *)localName, localName2, sizeof(name));
    });
    length = localLength;
    size = localSize;
    type = localType;
    memcpy(name, (void *)localName, sizeof(name));
  }

  Local<Object> activeInfo = Nan::New<Object>();
  activeInfo->Set(JS_STR("size"), JS_INT(size));
  activeInfo->Set(JS_STR("type"), JS_INT((int)type));
  activeInfo->Set(JS_STR("name"), JS_STR(name, length));

  info.GetReturnValue().Set(activeInfo);
}

NAN_METHOD(GetAttachedShaders) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();
  GLuint shaders[1024];
  GLsizei count;

  if (isUiThread) {
    glGetAttachedShaders(program, sizeof(shaders) / sizeof(shaders[0]), &count, shaders);
  } else {
    volatile GLuint localShaders[sizeof(shaders) / sizeof(shaders[0])];
    volatile GLsizei localCount;
    blockUiHard([&]() {
      GLuint localShaders2[sizeof(shaders) / sizeof(shaders[0])];
      GLsizei localCount2;
      glGetAttachedShaders(program, sizeof(shaders) / sizeof(shaders[0]), &localCount2, localShaders2);
      memcpy((void *)localShaders, localShaders2, sizeof(shaders));
      localCount = localCount2;
    });
    memcpy(shaders, (void *)localShaders, sizeof(shaders));
    count = localCount;
  }

  Local<Array> shadersArr = Nan::New<Array>(count);
  for(int i=0;i<count;i++) {
    shadersArr->Set(i, JS_INT((int)shaders[i]));
  }

  info.GetReturnValue().Set(shadersArr);
}

NAN_METHOD(GetParameter) {
  Nan::HandleScope scope;

  GLenum name = info[0]->Int32Value();

  switch(name) {
  case GL_BLEND:
  case GL_CULL_FACE:
  case GL_DEPTH_TEST:
  case GL_DEPTH_WRITEMASK:
  case GL_DITHER:
  case GL_POLYGON_OFFSET_FILL:
  case GL_SAMPLE_COVERAGE_INVERT:
  case GL_SCISSOR_TEST:
  case GL_STENCIL_TEST:
  case 0x9240 /* UNPACK_FLIP_Y_WEBGL */:
  case 0x9241 /* UNPACK_PREMULTIPLY_ALPHA_WEBGL*/:
  {
    // return a boolean
    GLboolean params;
    if (isUiThread) {
      glGetBooleanv(name, &params);
    } else {
      volatile GLboolean localParams;
      blockUiHard([&]() {
        GLboolean localParams2;
        glGetBooleanv(name, &localParams2);
        localParams = localParams2;
      });
      params = localParams;
    }
    info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(params)));
    break;
  }
  case GL_DEPTH_CLEAR_VALUE:
  case GL_LINE_WIDTH:
  case GL_POLYGON_OFFSET_FACTOR:
  case GL_POLYGON_OFFSET_UNITS:
  case GL_SAMPLE_COVERAGE_VALUE:
  {
    // return a float
    GLfloat params;
    if (isUiThread) {
      glGetFloatv(name, &params);
    } else {
      volatile GLfloat localParams;
      blockUiHard([&]() {
        GLfloat localParams2;
        glGetFloatv(name, &localParams2);
        localParams = localParams2;
      });
      params = localParams;
    }
    info.GetReturnValue().Set(JS_FLOAT(params));
    break;
  }
  case GL_RENDERER:
  case GL_SHADING_LANGUAGE_VERSION:
  case GL_VENDOR:
  case GL_EXTENSIONS:
  {
    // return a string
    char *params;
    if (isUiThread) {
      params = (char *)glGetString(name);
    } else {
      char * volatile localParams;
      blockUiHard([&]() {
        localParams = (char *)glGetString(name);
      });
      params = localParams;
    }
    
    if (params != NULL) {
      info.GetReturnValue().Set(JS_STR(params));
    } else {
      info.GetReturnValue().Set(Nan::Undefined());
    }
    
    break;
  }
  case GL_VERSION:
  {
    info.GetReturnValue().Set(JS_STR("WebGL 1"));

    break;
  }
  case GL_MAX_VIEWPORT_DIMS:
  {
    // return a int32[2]
    GLint params[2];
    if (isUiThread) {
      glGetIntegerv(name, params);
    } else {
      volatile GLint localParams[sizeof(params) / sizeof(params[0])];
      blockUiHard([&]() {
        GLint localParams2[sizeof(params) / sizeof(params[0])];
        glGetIntegerv(name, localParams2);
        memcpy((void *)localParams, localParams2, sizeof(params));
      });
      memcpy(params, (void *)localParams, sizeof(params));
    }

    Local<Array> arr = Nan::New<Array>(2);
    arr->Set(0,JS_INT(params[0]));
    arr->Set(1,JS_INT(params[1]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_SCISSOR_BOX:
  case GL_VIEWPORT:
  {
    // return a int32[4]
    GLint params[4];
    if (isUiThread) {
      glGetIntegerv(name, params);
    } else {
      volatile GLint localParams[sizeof(params) / sizeof(params[0])];
      blockUiHard([&]() {
        GLint localParams2[sizeof(params) / sizeof(params[0])];
        glGetIntegerv(name, localParams2);
        memcpy((void *)localParams, localParams2, sizeof(params));
      });
      memcpy(params, (void *)localParams, sizeof(params));
    }

    Local<Array> arr = Nan::New<Array>(4);
    arr->Set(0,JS_INT(params[0]));
    arr->Set(1,JS_INT(params[1]));
    arr->Set(2,JS_INT(params[2]));
    arr->Set(3,JS_INT(params[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_ALIASED_LINE_WIDTH_RANGE:
  case GL_ALIASED_POINT_SIZE_RANGE:
  case GL_DEPTH_RANGE:
  {
    // return a float[2]
    GLfloat params[2];
    if (isUiThread) {
      glGetFloatv(name, params);
    } else {
      volatile GLfloat localParams[sizeof(params) / sizeof(params[0])];
      blockUiHard([&]() {
        GLfloat localParams2[sizeof(params) / sizeof(params[0])];
        glGetFloatv(name, localParams2);
        memcpy((void *)localParams, localParams2, sizeof(params));
      });
      memcpy(params, (void *)localParams, sizeof(params));
    }

    Local<Array> arr = Nan::New<Array>(2);
    arr->Set(0,JS_FLOAT(params[0]));
    arr->Set(1,JS_FLOAT(params[1]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_BLEND_COLOR:
  case GL_COLOR_CLEAR_VALUE:
  {
    // return a float[4]
    GLfloat params[4];
    if (isUiThread) {
      glGetFloatv(name, params);
    } else {
      volatile GLfloat localParams[sizeof(params) / sizeof(params[0])];
      blockUiHard([&]() {
        GLfloat localParams2[sizeof(params) / sizeof(params[0])];
        glGetFloatv(name, localParams2);
        memcpy((void *)localParams, localParams2, sizeof(params));
      });
      memcpy(params, (void *)localParams, sizeof(params));
    }

    Local<Array> arr = Nan::New<Array>(4);
    arr->Set(0,JS_FLOAT(params[0]));
    arr->Set(1,JS_FLOAT(params[1]));
    arr->Set(2,JS_FLOAT(params[2]));
    arr->Set(3,JS_FLOAT(params[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_COLOR_WRITEMASK:
  {
    // return a boolean[4]
    GLboolean params[4];
    if (isUiThread) {
      glGetBooleanv(name, params);
    } else {
      volatile GLboolean localParams[sizeof(params) / sizeof(params[0])];
      blockUiHard([&]() {
        GLboolean localParams2[sizeof(params) / sizeof(params[0])];
        glGetBooleanv(name, localParams2);
        memcpy((void *)localParams, localParams2, sizeof(params));
      });
      memcpy(params, (void *)localParams, sizeof(params));
    }

    Local<Array> arr = Nan::New<Array>(4);
    arr->Set(0,JS_BOOL(params[0]==1));
    arr->Set(1,JS_BOOL(params[1]==1));
    arr->Set(2,JS_BOOL(params[2]==1));
    arr->Set(3,JS_BOOL(params[3]==1));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_ARRAY_BUFFER_BINDING:
  case GL_CURRENT_PROGRAM:
  case GL_ELEMENT_ARRAY_BUFFER_BINDING:
  case GL_FRAMEBUFFER_BINDING:
  case GL_RENDERBUFFER_BINDING:
  case GL_TEXTURE_BINDING_2D:
  case GL_TEXTURE_BINDING_CUBE_MAP:
  {
    GLint params;
    if (isUiThread) {
      glGetIntegerv(name, &params);
    } else {
      volatile GLint localParams;
      blockUiHard([&]() {
        GLint localParams2;
        glGetIntegerv(name, &localParams2);
        localParams = localParams2;
      });
      params = localParams;
    }

    info.GetReturnValue().Set(JS_INT(params));
    break;
  }
  default: {
    // return a long
    GLint params;
    if (isUiThread) {
      glGetIntegerv(name, &params);
    } else {
      volatile GLint localParams;
      blockUiHard([&]() {
        GLint localParams2;
        glGetIntegerv(name, &localParams2);
        localParams = localParams2;
      });
      params = localParams;
    }

    info.GetReturnValue().Set(JS_INT(params));
  }
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetBufferParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum pname = info[1]->Int32Value();
  GLint params;

  if (isUiThread) {
    glGetBufferParameteriv(target, pname, &params);
  } else {
    volatile GLint localParams;
    blockUiHard([&]() {
      GLint localParams2;
      glGetBufferParameteriv(target, pname, &localParams2);
      localParams = localParams2;
    });
    params = localParams;
  }

  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(GetFramebufferAttachmentParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum attachment = info[1]->Int32Value();
  GLenum pname = info[2]->Int32Value();
  GLint params;

  if (isUiThread) {
    glGetFramebufferAttachmentParameteriv(target,attachment, pname, &params);
  } else {
    volatile GLint localParams;
    blockUiHard([&]() {
      GLint localParams2;
      glGetFramebufferAttachmentParameteriv(target, attachment, pname, &localParams2);
      localParams = localParams2;
    });
    params = localParams;
  }

  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(GetProgramInfoLog) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();
  char Error[1024];
  int Len;

  if (isUiThread) {
    glGetProgramInfoLog(program, sizeof(Error), &Len, Error);
  } else {
    volatile char localError[sizeof(Error) / sizeof(Error[0])];
    volatile int localLen;
    blockUiHard([&]() {
      char localError2[sizeof(Error) / sizeof(Error[0])];
      int localLen2;
      glGetProgramInfoLog(program, sizeof(Error), &localLen2, localError2);
      memcpy((void *)localError, localError2, sizeof(Error));
      localLen = localLen2;
    });
    memcpy(Error, (void *)localError, sizeof(Error));
    Len = localLen;
  }

  info.GetReturnValue().Set(JS_STR(Error, Len));
}

NAN_METHOD(GetRenderbufferParameter) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int value;

  if (isUiThread) {
    glGetRenderbufferParameteriv(target, pname, &value);
  } else {
    volatile int localValue;
    blockUiHard([&]() {
      int localValue2;
      glGetRenderbufferParameteriv(target, pname, &localValue2);
      localValue2 = localValue2;
    });
    value = localValue;
  }

  info.GetReturnValue().Set(JS_INT(value));
}

NAN_METHOD(GetUniform) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();
  GLint location = info[1]->Int32Value();
  if (location < 0) info.GetReturnValue().Set(Nan::Undefined());
  float data[16]; // worst case scenario is 16 floats

  if (isUiThread) {
    glGetUniformfv(program, location, data);
  } else {
    volatile float localData[sizeof(data) / sizeof(data[0])];
    blockUiHard([&]() {
      float localData2[sizeof(data) / sizeof(data[0])];
      glGetUniformfv(program, location, localData2);
      memcpy((void *)localData, localData2, sizeof(data));
    });
    memcpy(data, (void *)localData, sizeof(data));
  }

  Local<Array> arr=Nan::New<Array>(16);
  for(int i=0;i<16;i++) {
    arr->Set(i,JS_FLOAT(data[i]));
  }

  info.GetReturnValue().Set(arr);
}

NAN_METHOD(GetVertexAttrib) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Int32Value();
  GLuint pname = info[1]->Int32Value();
  GLint value;

  switch (pname) {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      if (isUiThread) {
        glGetVertexAttribiv(index, pname, &value);
      } else {
        volatile GLint localValue;
        blockUiHard([&]() {
          GLint localValue2;
          glGetVertexAttribiv(index, pname, &localValue2);
          localValue = localValue2;
        });
        value = localValue;
      }
      info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value)));
      break;
    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      if (isUiThread) {
        glGetVertexAttribiv(index, pname, &value);
      } else {
        volatile GLint localValue;
        blockUiHard([&]() {
          GLint localValue2;
          glGetVertexAttribiv(index, pname, &localValue2);
          localValue = localValue2;
        });
        value = localValue;
      }
      info.GetReturnValue().Set(JS_INT(value));
      break;
    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      if (isUiThread) {
        glGetVertexAttribiv(index, pname, &value);
      } else {
        volatile GLint localValue;
        blockUiHard([&]() {
          GLint localValue2;
          glGetVertexAttribiv(index, pname, &localValue2);
          localValue = localValue2;
        });
        value = localValue;
      }
      info.GetReturnValue().Set(JS_INT(value));
      break;
    case GL_CURRENT_VERTEX_ATTRIB: {
      float vextex_attribs[4];
      if (isUiThread) {
        glGetVertexAttribfv(index, pname, vextex_attribs);
      } else {
        volatile float localVertexAttribs[sizeof(vextex_attribs) / sizeof(vextex_attribs[0])];
        blockUiHard([&]() {
          float localVertexAttribs2[sizeof(vextex_attribs) / sizeof(vextex_attribs[0])];
          glGetVertexAttribfv(index, pname, localVertexAttribs2);
          memcpy((void *)localVertexAttribs, localVertexAttribs2, sizeof(vextex_attribs));
        });
        memcpy(vextex_attribs, (void *)localVertexAttribs, sizeof(vextex_attribs));
      }
      Local<Array> arr = Nan::New<Array>(4);
      arr->Set(0,JS_FLOAT(vextex_attribs[0]));
      arr->Set(1,JS_FLOAT(vextex_attribs[1]));
      arr->Set(2,JS_FLOAT(vextex_attribs[2]));
      arr->Set(3,JS_FLOAT(vextex_attribs[3]));
      info.GetReturnValue().Set(arr);
      break;
    }
    default:
      Nan::ThrowError("GetVertexAttrib: Invalid Enum");
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetSupportedExtensions) {
  Nan::HandleScope scope;

  char *extensions;
  if (isUiThread) {
    extensions = (char *)glGetString(GL_EXTENSIONS);
  } else {
    char * volatile localExtensions;
    blockUiHard([&]() {
      localExtensions = (char *)glGetString(GL_EXTENSIONS);
    });
    extensions = localExtensions;
  }

  info.GetReturnValue().Set(JS_STR(extensions));
}

// TODO GetExtension(name) return the extension name if found, should be an object...
NAN_METHOD(GetExtension) {
  Nan::HandleScope scope;

  String::Utf8Value name(info[0]);
  char *sname = *name;

  if (
    strcmp(sname, "OES_texture_float") == 0 ||
    strcmp(sname, "OES_texture_float_linear") == 0 ||
    strcmp(sname, "OES_texture_half_float") == 0 ||
    strcmp(sname, "OES_texture_half_float_linear") == 0 ||
    strcmp(sname, "OES_element_index_uint") == 0 ||
    strcmp(sname, "OES_standard_derivatives") == 0
  ) {
    info.GetReturnValue().Set(Object::New(Isolate::GetCurrent()));
  } else if (strcmp(sname, "WEBGL_depth_texture") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "UNSIGNED_INT_24_8_WEBGL"), Number::New(Isolate::GetCurrent(), GL_UNSIGNED_INT_24_8_OES));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "EXT_texture_filter_anisotropic") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "MAX_TEXTURE_MAX_ANISOTROPY_EXT"), Number::New(Isolate::GetCurrent(), GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "TEXTURE_MAX_ANISOTROPY_EXT"), Number::New(Isolate::GetCurrent(), GL_TEXTURE_MAX_ANISOTROPY_EXT));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "WEBGL_compressed_texture_s3tc") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGB_S3TC_DXT1_EXT"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGB_S3TC_DXT1_EXT));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_S3TC_DXT1_EXT"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_S3TC_DXT1_EXT));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_S3TC_DXT3_EXT"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_S3TC_DXT3_EXT));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_S3TC_DXT5_EXT"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_S3TC_DXT5_EXT));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "WEBGL_compressed_texture_pvrtc") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGB_PVRTC_4BPPV1_IMG"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_PVRTC_4BPPV1_IMG"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGB_PVRTC_2BPPV1_IMG"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_PVRTC_2BPPV1_IMG"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "WEBGL_compressed_texture_etc1") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGB_ETC1_WEBGL"), Number::New(Isolate::GetCurrent(), GL_ETC1_RGB8_OES));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "ANGLE_instanced_arrays") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE"), Number::New(Isolate::GetCurrent(), GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE));
    Nan::SetMethod(result, "drawArraysInstancedANGLE", webgl::DrawArraysInstancedANGLE);
    Nan::SetMethod(result, "drawElementsInstancedANGLE", webgl::DrawElementsInstancedANGLE);
    Nan::SetMethod(result, "vertexAttribDivisorANGLE", webgl::VertexAttribDivisorANGLE);
    info.GetReturnValue().Set(result);
  } else {
    info.GetReturnValue().Set(Null(Isolate::GetCurrent()));
  }
}

NAN_METHOD(CheckFramebufferStatus) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLint ret;

  if (isUiThread) {
    ret = glCheckFramebufferStatus(target);
  } else {
    volatile GLint localRet;
    blockUiHard([&]() {
      localRet = glCheckFramebufferStatus(target);
    });
    ret = localRet;
  }

  info.GetReturnValue().Set(JS_INT(ret));
}

/* struct GLObj {
  GLObjectType type;
  GLuint obj;
  GLObj(GLObjectType type, GLuint obj) {
    this->type=type;
    this->obj=obj;
  }
};

vector<GLObj*> globjs;
static bool atExit=false;

void registerGLObj(GLObjectType type, GLuint obj) {
  globjs.push_back(new GLObj(type,obj));
}


void unregisterGLObj(GLuint obj) {
  if(atExit) return;

  vector<GLObj*>::iterator it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    if(globj->obj==obj) {
      delete globj;
      globjs.erase(it);
      break;
    }
    ++it;
  }
}

void AtExit() {
  atExit=true;
  //glFinish();

  vector<GLObj*>::iterator it;

  #ifdef LOGGING
  cout<<"WebGL AtExit() called"<<endl;
  cout<<"  # objects allocated: "<<globjs.size()<<endl;
  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *obj=*it;
    cout<<"[";
    switch(obj->type) {
    case GLOBJECT_TYPE_BUFFER: cout<<"buffer"; break;
    case GLOBJECT_TYPE_FRAMEBUFFER: cout<<"framebuffer"; break;
    case GLOBJECT_TYPE_PROGRAM: cout<<"program"; break;
    case GLOBJECT_TYPE_RENDERBUFFER: cout<<"renderbuffer"; break;
    case GLOBJECT_TYPE_SHADER: cout<<"shader"; break;
    case GLOBJECT_TYPE_TEXTURE: cout<<"texture"; break;
    };
    cout<<": "<<obj->obj<<"] ";
    ++it;
  }
  cout<<endl;
  #endif

  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    GLuint obj=globj->obj;

    switch(globj->type) {
    case GLOBJECT_TYPE_PROGRAM:
      #ifdef LOGGING
      cout<<"  Destroying GL program "<<obj<<endl;
      #endif
      glDeleteProgram(obj);
      break;
    case GLOBJECT_TYPE_BUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL buffer "<<obj<<endl;
      #endif
      glDeleteBuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_FRAMEBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL frame buffer "<<obj<<endl;
      #endif
      glDeleteFramebuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_RENDERBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL render buffer "<<obj<<endl;
      #endif
      glDeleteRenderbuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_SHADER:
      #ifdef LOGGING
      cout<<"  Destroying GL shader "<<obj<<endl;
      #endif
      glDeleteShader(obj);
      break;
    case GLOBJECT_TYPE_TEXTURE:
      #ifdef LOGGING
      cout<<"  Destroying GL texture "<<obj<<endl;
      #endif
      glDeleteTextures(1,&obj);
      break;
    default:
      #ifdef LOGGING
      cout<<"  Unknown object "<<obj<<endl;
      #endif
      break;
    }
    delete globj;
    ++it;
  }

  globjs.clear();
} */

} // end namespace webgl
