#ifndef IMAGEBITMAP_H_
#define IMAGEBITMAP_H_

#include <v8.h>
#include <nan/nan.h>
#include <Context.h>
#include <ImageData.h>

using namespace v8;
using namespace node;

class CanvasRenderingContext2D;
class Image;

#define JS_STR(...) Nan::New<v8::String>(__VA_ARGS__).ToLocalChecked()
#define JS_INT(val) Nan::New<v8::Integer>(val)
#define JS_FLOAT(val) Nan::New<v8::Number>(val)
#define JS_BOOL(val) Nan::New<v8::Boolean>(val)

class ImageBitmap : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  int GetWidth();
  int GetHeight();

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(DataGetter);
  static NAN_METHOD(CreateImageBitmap);

  ImageBitmap(Image *image);
  virtual ~ImageBitmap();

private:
  canvas::ImageData *imageData;

  friend class CanvasRenderingContext2D;
};

#include "canvas.h"

#endif
