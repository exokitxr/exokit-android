console.log('JNI boot html5.js 1');

const jsdom = require('jsdom');
const HTMLCanvasElement = require('jsdom/lib/jsdom/living/nodes/HTMLCanvasElement-impl');
HTMLCanvasElement.implementation.prototype._getCanvas = () => ({
  getContext: () => {
    return {};
  },
});
const THREE = require('three-zeo');

const {window} = new jsdom.JSDOM();
global.window = window;
const {document} = window;

const canvas = document.createElement('canvas');
/* const renderer = new THREE.WebGLRenderer({
  canvas,
  context: canvas.getContext('webgl'),
  antialias: true,
});
// renderer.setSize(canvas.width, canvas.height);
renderer.setClearColor(0xffffff, 1);

const scene = new THREE.Scene();

const _makeCamera = () => {
  const camera = new THREE.PerspectiveCamera(90, canvasWidth / canvasHeight, 0.1, 1000);
  camera.position.set(0, 0, 2);
  camera.lookAt(new THREE.Vector3(0, 0, 0));
  return camera;
};
let camera = _makeCamera();
scene.add(camera);

const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
directionalLight.position.set(1, 1, 1);
scene.add(directionalLight);

const boxMesh = (() => {
  const geometry = new THREE.BoxBufferGeometry(1, 1, 1);
  const material = new THREE.MeshPhongMaterial({
    color: 0xFF0000,
  });
  return new THREE.Mesh(geometry, material);
})();
scene.add(boxMesh); */

global.onSurfaceCreated = () => {
  console.log('js onSurfaceCreated');
};
global.onSurfaceCreated = (width, height) => {
  console.log('js onSurfaceChanged', {width, height});
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

console.log('JNI boot html5.js 2');
