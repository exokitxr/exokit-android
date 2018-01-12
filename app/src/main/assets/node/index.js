const path = require('path');
const fs = require('fs');
const http = require('http');
const cowsay = require('cowsay');

const javaApi = (() => {
  const port = Number(process.argv[process.argv.length - 1]);

  return {
    loadUrl: u => new Promise((accept, reject) => {
      console.log('node load url 1');

      const req = http.request({
        method: 'POST',
        host: '127.0.0.1',
        port,
      }, res => {
        console.log('node load url 3', res.statusCode);

        if (res.statusCode >= 200 && res.statusCode < 300) {
          accept();
        } else {
          reject(new Error('server request got invalid status code'));
        }
      });
      req.end(JSON.stringify({
        method: 'loadUrl',
        args: u,
      }) + '\n');
      req.on('error', err => {
        reject(err);
      });

      console.log('node load url 2');
    }),
  };
})();

console.log(cowsay.say({
  text : "I'm a moooodule",
  e : "oO",
  T : "U "
}));

console.log("hello world");
console.log('argv', process.argv);

const server = http.createServer((req, res) => {
  res.setHeader('Content-Type', 'text/html')

  const rs = fs.createReadStream(path.join(__dirname, 'public', 'index.html'));
  rs.pipe(res);
  rs.on('error', err => {
    res.statusCode = 500;
    res.end(err.stack);
  });
});

server.listen(0, () => {
  console.log('node load 1');

  javaApi.loadUrl('http://localhost:' + server.address().port)
    .then(() => {
      console.log('node load 2');
    })
    .catch(err => {
      console.warn(err);
    });
});
