const child = require("child_process");
const fs = require("fs");

let defaultCompiler = "gcc";
let tempDir;
let defaultLinkFlags = "%.o"
let defaultOutput = "$name"

let linkCache = "";


module.exports = {
    
    /**
     * Preprocess the buildscript.
     * If it contains compile, assemble and link steps, it does nothing
     * If it contains compile but no build, it implicitly adds it with the default arguments.
     * @param {object} buildscript the JSON buildscript object
     */
    extensionC: function(buildscript) {
        console.log("c.js processing buildscript");
        if(!("link" in buildscript.build)) {
            console.log("Inserting link step for gcc");
            buildscript.build["link"] = [defaultLinkFlags];
        }

        if(!("output" in buildscript.build)) {
            console.log("Inserting output step");
            buildscript.build["output"] = [defaultOutput];
        }

        tempDir = process.cwd() + "/bsTemp/";
        
        if(!fs.existsSync(tempDir)) {
            console.log("Creating temporary dir for object files");
            fs.mkdirSync(tempDir);
        }

        
    },

    extensionH: function() {},
    extensionO: function() {},
    extensionLD: function() {},

    /**
     * Called when it is time to execute the compile step.
     * It is passed the array of strings associated with the step in the buildscript.
     * @param  {...string} params the list of strings
     */
    stepCompile: function(...params) {
        let cwd = process.cwd().replace(/\\/g, "/");
        let compileFlags = "";
        // Check whether we're doubly listed
        if(Array.isArray(params[0])) {
            params = params[0];
        }

        for(var param of params) {
            console.log("stepCompile: param", param);
            if(param.startsWith("-")) {
                compileFlags = param;
            } else {
                let binPath = param.substr(param.lastIndexOf(cwd) + cwd.length + 1);
                binPath = binPath.substr(0, binPath.lastIndexOf("."));
                binPath = cwd + "/bsTemp/" + binPath;

                // generate the file structure for it to go into
                fs.mkdirSync(binPath.substr(0, binPath.lastIndexOf("/")), {recursive: true});
                if(param.substr(param.lastIndexOf(".") + 1) != "c" 
                    && param.substr(param.lastIndexOf(".") + 1) != "s") {

                    fs.copyFileSync(param, binPath + ".o");
                } else {
                    let compilerCommand =
                        defaultCompiler + 
                        " -c " + param +
                        " -o " + binPath + ".o "
                        + compileFlags;
                        
                    child.execSync(compilerCommand);
                }
                
            }
        }
    },

    /**
     * Take in the compiled binary object files,
     * prepare the link command, and save it for the link execution.
     * @param {...string} params the object files to link. 
     */
    stepLink: function(...params) {
        let linkerCommand = "";
        // Check whether we're doubly listed
        if(Array.isArray(params[0])) {
            params = params[0];
        }

        for(var param of params) {
            console.log("stepLink: param", param);
            linkerCommand += param + " ";
        }

        linkCache = linkerCommand;
    },

    /**
     * Take in the name of a file, execute the link command to save the output.
     * @param {string} output the name of the wanted file
     */
    stepOutput: function(output) {
        // Check whether we're doubly listed
        if(Array.isArray(output[0])) {
            output = output[0];
        }

        let linkerCommand = defaultCompiler + " -o " + process.cwd() + "/" + output + " " + linkCache;
        console.log("stepOutput: param", output, "command", linkerCommand);
        child.execSync(linkerCommand);

    },

    /**
     * Set the name of the compiler to be used.
     * @param {string} comp 
     */
    setCompiler: function(comp) {
        // Check whether we're doubly listed
        if(Array.isArray(comp[0])) {
            comp = comp[0];
        }
        defaultCompiler = comp;
        console.log("Set compiler to", comp);
    }

}