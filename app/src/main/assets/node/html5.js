console.log('node boot html start');

// BOOTSTRAP

const path = require('path');

// correct exec path autodetection
const libPath = process.argv[2];
process.argv[0] = process.execPath = path.join(libPath, 'node.so');
process.env['LD_LIBRARY_PATH'] = libPath;

// ENVIRONMENT

const exokit = require('exokit');
const {THREE} = exokit;

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
const _texImage2D = nativeGl.texImage2D;
nativeGl.texImage2D = function() {
  console.log('got teximage2d');
  _texImage2D.apply(this, arguments);
};
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

// CALLBACKS

global.onResize = (width, height) => {
  innerWidth = width;
  innerHeight = height;

  if (window) {
    window.innerWidth = innerWidth;
    window.innerHeight = innerHeight;
    window.emit('resize');
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
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();
const localMatrix3 = new THREE.Matrix4();
global.onDrawFrame = (viewMatrixFloat32Array, projectionMatrixFloat32Array, centerFloat32Array) => {
  if (window) {
    const matrixWorldInverse = localMatrix.fromArray(viewMatrixFloat32Array);
    const matrixWorld = localMatrix2.getInverse(matrixWorldInverse)
      .premultiply(
        localMatrix3.makeTranslation(-centerFloat32Array[0], -centerFloat32Array[1], -centerFloat32Array[2])
      );
    matrixWorld.toArray(viewMatrixFloat32Array);

    window.alignFrame(viewMatrixFloat32Array, projectionMatrixFloat32Array);
    window.tickAnimationFrame();
  }
};

// MAIN

let window = null;
let innerWidth = 1280;
let innerHeight = 1024;

exokit.fetch('http://192.168.0.13:8000/')
  .then(site => {
    console.log('node site loaded');

    window = site.window;
    window.innerWidth = innerWidth;
    window.innerHeight = innerHeight;
    window.navigator.setVRMode('ar');
    window.addEventListener('error', err => {
      console.warn('got error', err.error.stack);
    });
  });

process.on('uncaughtException', err => {
  console.warn(err.stack);
});
process.on('unhandledRejection', err => {
  console.warn(err.stack);
});

console.log('node boot html end');
