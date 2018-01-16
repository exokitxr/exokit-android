global.onSurfaceCreated = () => {
  console.log('js onSurfaceCreated');
};
global.onSurfaceChanged = (width, height) => {
  console.log('js onSurfaceChanged', {width, height});
  gl.viewport(0, 0, width, height);
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
