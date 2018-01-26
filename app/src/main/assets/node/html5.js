console.log('node boot html start');

// BOOTSTRAP

const path = require('path');

// correct exec path autodetection
const libPath = process.argv[2];
process.argv[0] = process.execPath = path.join(libPath, 'node.so');
process.env['LD_LIBRARY_PATH'] = libPath;

// ENVIRONMENT

const exokit = require('exokit');
// const THREE = require('three-zeo');

/* const {VERSION} = nativeGl;

nativeGl = {};
nativeGl.VERSION = VERSION; */
/* nativeGl.enable = () => {};
nativeGl.disable = () => {};
nativeGl.clear = () => {};
nativeGl.getExtension = () => null;
nativeGl.getParameter = id => {
  if (id === VERSION) {
    return 'WebGL 1';
  } else {
    return {};
  }
};
nativeGl.createTexture = () => {};
nativeGl.bindTexture = () => {};
nativeGl.texParameteri = () => {};
nativeGl.texImage2D = () => {};
nativeGl.clearColor = () => {};
nativeGl.clearDepth = () => {};
nativeGl.clearStencil = () => {};
nativeGl.depthFunc = () => {};
nativeGl.frontFace = () => {};
nativeGl.cullFace = () => {};
nativeGl.blendEquationSeparate = () => {};
nativeGl.blendFuncSeparate = () => {};
nativeGl.blendEquation = () => {};
nativeGl.blendFunc = () => {};
const _viewport = nativeGl.viewport;
nativeGl.viewport = function() {
  console.log('gl viewport', arguments, new Error().stack);
  _viewport.apply(this, arguments);
}; */

/* const {window} = exokit();
const {document, fetch} = window; */

const windows = [];
let innerWidth = 1280;
let innerHeight = 1024;

// CALLBACKS

/* global.onSurfaceCreated = () => {
  // console.log('js onSurfaceCreated');

  if (!animating) {
    animating = true;

    _startAnimation();
  }
}; */
global.onResize = (width, height) => {
  //  console.log('js onSurfaceChanged', {width, height});

  // gl.viewport(0, 0, width, height);

  innerWidth = width;
  innerHeight = height;

  for (let i = 0; i < windows.length; i++) {
    const window = windows[i];

    window.innerWidth = innerWidth;
    window.innerHeight = innerHeight;
    window.emit('resize');
  }

  // renderer.setSize(width, height);
};

// VR
global.onNewFrame = (headViewMatrixFloat32Array, headQuaternionFloat32Array) => {
  // console.log('js onNewFrame', headViewMatrixFloat32Array, headQuaternionFloat32Array);
};
global.onDrawEye = (eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array) => {
  // console.log('js onDrawEye', eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array);
};

// AR
// const localMatrix = new THREE.Matrix4();
global.onDrawFrame = (viewMatrixFloat32Array, projectionMatrixFloat32Array, centerFloat32Array) => {
  /* camera.matrixWorldInverse.fromArray(viewMatrixFloat32Array);
  camera.matrixWorld.getInverse(camera.matrixWorldInverse)
    .premultiply(localMatrix.makeTranslation(-centerFloat32Array[0], -centerFloat32Array[1], -centerFloat32Array[2]));
  camera.matrixWorldInverse.getInverse(camera.matrixWorld);

  camera.projectionMatrix.fromArray(projectionMatrixFloat32Array); */

  for (let i = 0; i < windows.length; i++) {
    windows[i].tickAnimationFrame();
  }
};

// MAIN

/* const skinJs = require('skin-js');
const skinJsPath = path.dirname(require.resolve('skin-vr'));
const skin = skinJs(THREE); */

const appUrl = 'http://192.168.0.13:8000/';
exokit.fetch(appUrl, {
  url: appUrl,
})
  .then(site => {
    console.log('node site loaded');

    const {window} = site;
    window.innerWidth = innerWidth;
    window.innerHeight = innerHeight;
    windows.push(window);
    window.addEventListener('error', err => {
      console.warn('got error', err.error.stack);
    });
  });

/* const canvas = document.createElement('canvas');
canvas.width = window.innerWidth;
canvas.height = window.innerHeight;
const gl = canvas.getContext('webgl');
let renderer = null;
let camera = null;

let animating = false;
const _startAnimation = () => {
  console.log('node html start animation');

  renderer = new THREE.WebGLRenderer({
    canvas,
    context: gl,
    antialias: true,
    alpha: true,
    preserveDrawingBuffer: true,
  });
  renderer.autoClear = false;
  // console.log('disable', gl.DEPTH_TEST);
  // gl.disable(gl.DEPTH_TEST);
  // console.log('disable', gl.CULL_FACE);
  // gl.disable(gl.CULL_FACE);
  // renderer.setSize(canvas.width, canvas.height);
  renderer.setClearColor(0x000000, 0);
  // renderer.setClearColor(0xFF0000, 0.5);

  const scene = new THREE.Scene();
  scene.autoUpdate = false;

  camera = new THREE.PerspectiveCamera(90, canvas.width / canvas.height, 0.1, 100);
  camera.position.set(2, 2, 2);
  camera.updateMatrixWorld();
  camera.lookAt(new THREE.Vector3(0, 0, 0));
  camera.matrixAutoUpdate = false;
  scene.add(camera);

  const ambientLight = new THREE.AmbientLight(0xffffff, 0.5);
  scene.add(ambientLight);

  const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
  directionalLight.position.set(3, 3, 3);
  directionalLight.updateMatrixWorld();
  scene.add(directionalLight);

  const _requestImage = src => new Promise((accept, reject) => {
    const img = new Image();
    img.onload = () => {
      console.log('image onload', img.data.length);

      accept(img);
    };
    img.onerror = err => {
      console.log('image onerror', err);

      reject(err);
    };
    img.src = src;
  });

  const skinMesh = (() => {
    const object = new THREE.Object3D();

    _requestImage(path.join(skinJsPath, 'lib', 'img', 'female.png'))
      .then(skinImg => {
        const mesh = skin(skinImg);
        object.add(mesh);
      });

    return object;
  })();
  scene.add(skinMesh);

  const _recurse = () => {
    renderer.state.reset();

    renderer.render(scene, camera);

    window.requestAnimationFrame(_recurse);
  };
  _recurse();
}; */

console.log('node boot html end');
