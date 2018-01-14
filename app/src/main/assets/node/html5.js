global.onSurfaceCreated = () => {
  console.log('js onSurfaceCreated');
};
global.onSurfaceCreated = (w, h) => {
  console.log('js onSurfaceChanged', {w, h});
};
global.onDrawFrame = () => {
  // console.log('js onDrawFrame');
};
