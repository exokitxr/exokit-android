#include "imageBitmap.h"

using namespace v8;
using namespace node;
// using namespace std;

Handle<Object> ImageBitmap::Initialize(Isolate *isolate) {
  v8::EscapableHandleScope scope(isolate);

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("ImageBitmap"));

  // prototype
  // Nan::SetPrototypeMethod(ctor, "save",save);// NODE_SET_PROTOTYPE_METHOD(ctor, "save", save);
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetAccessor(proto,JS_STR("width"), WidthGetter);
  Nan::SetAccessor(proto,JS_STR("height"), HeightGetter);
  Nan::SetAccessor(proto,JS_STR("data"), DataGetter);

  ctor->Set(JS_STR("createImageBitmap"), Nan::New<Function>(CreateImageBitmap));

  // Nan::SetAccessor(proto,JS_STR("src"), SrcGetter, SrcSetter);
  // Nan::Set(target, JS_STR("Image"), ctor->GetFunction());

  // constructor_template.Reset(Isolate::GetCurrent(), ctor->GetFunction());

  return scope.Escape(ctor->GetFunction());
}

unsigned int ImageBitmap::GetWidth() {
  return imageData->getWidth();
}

unsigned int ImageBitmap::GetHeight() {
  return imageData->getHeight();
}

unsigned int ImageBitmap::GetNumChannels() {
  return imageData->getNumChannels();
}

unsigned char *ImageBitmap::GetData() {
  return imageData->getData();
}

NAN_METHOD(ImageBitmap::New) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(Local<Object>::Cast(info[0]));
  ImageBitmap *imageBitmap = new ImageBitmap(image);
  imageBitmap->Wrap(info.This());
  // registerImage(image);
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(ImageBitmap::WidthGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());

  info.GetReturnValue().Set(JS_INT(imageBitmap->GetWidth()));
}

NAN_GETTER(ImageBitmap::HeightGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());

  info.GetReturnValue().Set(JS_INT(imageBitmap->GetHeight()));
}

NAN_GETTER(ImageBitmap::DataGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());
  unsigned int width = imageBitmap->GetWidth();
  unsigned int height = imageBitmap->GetHeight();
  // unsigned int numChannels = imageBitmap->GetNumChannels();
  // std::cout << "imagebitmap data getter " << (void *)imageBitmap->GetData() << " : " << width << " : " << height << " : " << numChannels << "\n";
  Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), imageBitmap->GetData(), width * height * 4);
  Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());

  info.GetReturnValue().Set(uint8ClampedArray);
}

NAN_METHOD(ImageBitmap::CreateImageBitmap) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(Local<Object>::Cast(info.This()));

  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> isolateContext = isolate->GetCurrentContext();
  Local<Object> global = isolateContext->Global();
  Local<Function> imageBitmapConstructor = Local<Function>::Cast(global->Get(JS_STR("ImageBitmap")));
  Local<Value> argv[] = {
    info[0],
  };
  Local<Object> imageBitmapObj = imageBitmapConstructor->NewInstance(sizeof(argv) / sizeof(argv[0]), argv);

  info.GetReturnValue().Set(imageBitmapObj);
}

ImageBitmap::ImageBitmap(Image *image) {
  imageData = new canvas::ImageData(image->image->getData());
}
ImageBitmap::~ImageBitmap () {
  delete imageData;
}
