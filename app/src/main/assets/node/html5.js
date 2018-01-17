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

const rafCbs = [];
window.requestAnimationFrame = fn => {
  rafCbs.push(fn);
};
window.clearAnimationFrame = fn => {
  const index = rafCbs.indexOf(fn);
  if (index !== -1) {
    rafCbs.splice(index, 1);
  }
};

const canvas = document.createElement('canvas');
canvas.width = window.innerWidth;
canvas.height = window.innerHeight;
const gl = canvas.getContext('webgl');

let animating = false;
const _startAnimation = () => {
  console.log('node html start animation');

  const renderer = new THREE.WebGLRenderer({
    canvas,
    context: gl,
    antialias: true,
  });
  // renderer.setSize(canvas.width, canvas.height);
  renderer.setClearColor(0xffffff, 1);

  const scene = new THREE.Scene();

  const _makeCamera = () => {
    const camera = new THREE.PerspectiveCamera(90, canvas.width / canvas.height, 0.1, 1000);
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
  scene.add(boxMesh);

  const _recurse = () => {
    renderer.render(scene, camera);

    window.requestAnimationFrame(_recurse);
  };
  _recurse();
};



// CALLBACKS

global.onSurfaceCreated = () => {
  console.log('js onSurfaceCreated');
};
global.onSurfaceChanged = (width, height) => {
  console.log('js onSurfaceChanged', {width, height});

  gl.viewport(0, 0, width, height);

  window.innerWidth = width;
  window.innerHeight = height;

  if (!animating) {
    animating = true;

    _startAnimation();
  }
};

// VR
global.onNewFrame = (headViewMatrixFloat32Array, headQuaternionFloat32Array) => {
  // console.log('js onNewFrame', headViewMatrixFloat32Array, headQuaternionFloat32Array);
};
global.onDrawEye = (eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array) => {
  // console.log('js onDrawEye', eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array);
};

// AR
const arState = {
  viewMatrix: new Float32Array(16),
  projectionMatrix: new Float32Array(16),
};
global.onDrawFrame = (viewMatrixFloat32Array, projectionMatrixFloat32Array) => {
  arState.viewMatrix.set(viewMatrixFloat32Array);
  arState.projectionMatrix.set(projectionMatrixFloat32Array);

  const localRafCbs = rafCbs.slice();
  rafCbs.length = 0;
  for (let i = 0; i < localRafCbs; i++) {
    localRafCbs[i]();
  }
};

console.log('node boot html end');
