console.log('node boot html start');

// ENVIRONMENT

const jsdom = require('jsdom');
const HTMLCanvasElement = require('jsdom/lib/jsdom/living/nodes/HTMLCanvasElement-impl');
const canvasImplementation = {
  getContext: () => nativeGl,
};
HTMLCanvasElement.implementation.prototype._getCanvas = () => canvasImplementation;
const THREE = require('three-zeo');

const {window} = new jsdom.JSDOM();
global.window = window;
const {document} = window;

const canvas = document.createElement('canvas');
const gl = canvas.getContext('webgl');



// CALLBACKS

global.onSurfaceCreated = () => {
  console.log('js onSurfaceCreated');
};
global.onSurfaceChanged = (width, height) => {
  console.log('js onSurfaceChanged', {width, height});
  gl.viewport(0, 0, width, height);
};

// VR
global.onNewFrame = (headViewMatrixFloat32Array, headQuaternionFloat32Array) => {
  // console.log('js onNewFrame', headViewMatrixFloat32Array, headQuaternionFloat32Array);
};
global.onDrawEye = (eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array) => {
  // console.log('js onDrawEye', eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array);
};

// AR
global.onDrawFrame = (viewMatrixFloat32Array, projectionMatrixFloat32Array) => {
  // console.log('js onDrawFrame', viewMatrixFloat32Array, projectionMatrixFloat32Array);
};

console.log('node boot html end');
