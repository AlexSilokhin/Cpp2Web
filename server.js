const http = require("http");
const fs = require("fs");
const path = require("path");

const port = 8080;
const root = __dirname;

http.createServer((req, res) => {
    let filePath = path.join(root, req.url === "/" ? "index.html" : req.url);

    fs.readFile(filePath, (err, data) => {
        if (err) {
            res.writeHead(404, { "Content-Type": "text/plain" });
            res.end("Not found");
            return;
        }

        let contentType = "application/octet-stream";
        if (filePath.endsWith(".html")) contentType = "text/html";
        else if (filePath.endsWith(".js")) contentType = "application/javascript";
        else if (filePath.endsWith(".wasm")) contentType = "application/wasm";

        res.writeHead(200, {
            "Content-Type": contentType,
            "Cross-Origin-Opener-Policy": "same-origin",
            "Cross-Origin-Embedder-Policy": "require-corp",
        });
        res.end(data);
    });
}).listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
});
