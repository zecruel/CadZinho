//npm install --save screenshot-desktop

const exec = require('child_process').execFile;
const screenshot = require("screenshot-desktop");

const subprocess = await exec('cadzinho.exe', function(err, data) {  
        console.log(err)
        console.log(data.toString());                       
    });

  screenshot({ filename: 'test.png' });


setTimeout(() => {
  subprocess.kill(); // Does not terminate the Node.js process in the shell.
}, 2000);
