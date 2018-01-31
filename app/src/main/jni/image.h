#ifndef IMAGE_H_
#define IMAGE_H_

#include <v8.h>
#include <nan/nan.h>
#include <Context.h>
#include <Image.h>
#include <ImageData.h>

using namespace v8;
using namespace node;

class CanvasRenderingContext2D;

#define JS_STR(...) Nan::New<v8::String>(__VA_ARGS__).ToLocalChecked()
#define JS_INT(val) Nan::New<v8::Integer>(val)
#define JS_FLOAT(val) Nan::New<v8::Number>(val)
#define JS_BOOL(val) Nan::New<v8::Boolean>(val)

class Image : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate, canvas::ContextFactory *canvasContextFactory);
  unsigned int GetWidth();
  unsigned int GetHeight();
  unsigned int GetNumChannels();
  unsigned char *GetData();
  bool Load(const unsigned char *buffer, size_t size);

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(DataGetter);
  static NAN_METHOD(LoadMethod);

  Image();
  virtual ~Image();

private:
  static canvas::ContextFactory *canvasContextFactory;

  canvas::Image *image;

  friend class CanvasRenderingContext2D;
  friend class ImageData;
  friend class ImageBitmap;
};

#include "canvas.h"

#endif
