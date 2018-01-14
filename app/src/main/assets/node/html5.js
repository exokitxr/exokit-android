global.onSurfaceCreated = () => {
  console.log('js onSurfaceCreated');
};
global.onSurfaceCreated = (w, h) => {
  console.log('js onSurfaceChanged', {w, h});
};

global.onNewFrame = (headViewMatrixFloat32Array, headQuaternionFloat32Array) => {
  // console.log('js onNewFrame', headViewMatrixFloat32Array, headQuaternionFloat32Array);
};
global.onDrawEye = (eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array) => {
  // console.log('js onDrawEye', eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array);
};

global.onDrawFrame = (viewMatrixFloat32Array, projectionMatrixFloat32Array) => {
  // console.log('js onDrawFrame', viewMatrixFloat32Array, projectionMatrixFloat32Array);
};
