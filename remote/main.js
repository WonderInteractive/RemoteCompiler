const http = require("http");
const fs = require('fs');
const { promisify } = require('util');
const exec = promisify(require('child_process').exec)

const host = 'localhost';
const port = 30000;
function sleep(ms) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms);
  });
}
const requestListener = async function (req, res) {
    const cmd = req.headers['cmd'];
    if (cmd === 'ping') {
        res.writeHead(200);
        res.end('1');
    }
    let data = '';
    req.on('data', chunk => {
        data += chunk;
    });
    req.on('end', async () => {
        //await sleep(5000)
        console.log(cmd, data); // 'Buy the milk'
        fs.writeFileSync('temp.cpp', data);
        const nameOutput = await exec("\"C:/Program Files (x86)/Microsoft Visual Studio\\2019\\BuildTools\\VC\\Tools\\MSVC\\14.29.30133\\bin\\Hostx64\\x86\\cl.exe\" temp.cpp /c");
        
        //read temp.obj
        const buf3 = fs.readFileSync("temp.obj");
        console.log('out', buf3); // 'Buy the milk'

        res.end('response');
    });
}

const server = http.createServer(requestListener);
server.listen(port, host, () => {
    console.log(`Server is running on http://${host}:${port}`);
});