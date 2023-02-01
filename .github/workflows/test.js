//npm install --save screenshot-desktop

const exec = require('child_process').execFile;
const screenshot = require("screenshot-desktop");

const subprocess = exec('cadzinho.exe', function(err, data) {  
        console.log(err)
        console.log(data.toString());                       
    });

setTimeout(() => {
  screenshot({ filename: 'test.png' });
}, 1000);

setTimeout(() => {
  subprocess.kill(); // Does not terminate the Node.js process in the shell.
}, 2000);
