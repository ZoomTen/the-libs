//Image the project

const core = require('@actions/core');
const exec = require('@actions/exec');
const io = require('@actions/io');
const tc = require('@actions/tool-cache');
const process = require('process');
const fs = require('fs');

(async () => {
    if (process.platform === 'linux') {
        let extraPlugins = core.getInput("extra-plugins-linux");
        if (extraPlugins !== "") {
            extraPlugins = `,${extraPlugins}`;
        }
        
        const ldepqt = await tc.downloadTool("https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage");
        await exec.exec(`chmod a+x ${ldepqt}`);
        
        let ldepqtArgs = [];
        
        let applications = fs.readdirSync(`${process.env["HOME"]}/appdir/usr/share/applications/`);
        for (let application of applications) {
            ldepqtArgs.push(`${process.env["HOME"]}/appdir/usr/share/applications/${application}`);
        }
        
        ldepqtArgs.push("-appimage");
        ldepqtArgs.push(`-extra-plugins=iconengines/libqsvgicon.so,imageformats/libqsvg.so${extraPlugins}`);
        if (await exec.exec(ldepqt, ldepqtArgs) != 0) {
            core.setFailed("linuxdeployqt failed.");
            return;
        }
        
        let appimages = fs.readdirSync(process.cwd());
        for (let appimage of appimages) {
            if (appimage.endsWith(".AppImage")) {
                await io.cp(`${process.cwd()}/${appimage}`, `${process.env["HOME"]}/${core.getInput("image-name-linux")}`);
            }
        }
        
        core.setOutput("image-path", `${process.env["HOME"]}/${core.getInput("image-name-linux")}`);
        core.setOutput("asset-name", core.getInput("image-name-linux"));
        core.setOutput("asset-content-type", "application/x-appimage");
    } else if (process.platform === 'darwin') {        
        const appdmg = require('appdmg');
        
        let bundlePath = core.getInput("app-bundle-mac");
        if (bundlePath === "") {
            core.setFailed("Not running on a supported platform.");
            return;
        }
        
        bundlePath = `${process.cwd()}/build/${bundlePath}`;
        let executableName = bundlePath.replace(".app", "");
        if (executableName.includes("/")) executableName = executableName.substr(executableName.lastIndexOf("/") + 1);

        let installDir = "";
        await exec.exec("qmake", ["-query", "QT_INSTALL_LIBS"], {
            listeners: {
                stdout: (data) => {
                    installDir += data.toString()
                }
            }
        });
        
        //Embed libraries
        let embedLibs = core.getInput("embed-libraries-mac").split(" ");
        
        let macDeployQtArgs = [bundlePath];
        
        let libDir = `${bundlePath}/Contents/Libraries`;
        await io.rmRF(libDir);
        await io.mkdirP(libDir);
        
        for (let lib of embedLibs) {
            if (lib == "") continue;
            
            await exec.exec('cp', [`/usr/local/lib/lib${lib}.dylib`, `${libDir}/lib${lib}.1.dylib`]);
            await exec.exec("install_name_tool", ["-change", `lib${lib}.1.dylib`, `@executable_path/../Libraries/lib${lib}.1.dylib`, `${bundlePath}/Contents/MacOS/${executableName}`])
            if (lib != "the-libs") {
                await exec.exec("install_name_tool", ["-change", `the-libs.framework/Versions/1/the-libs`, `@executable_path/../Frameworks/the-libs.framework/Versions/Current/the-libs`, `${libDir}/lib${lib}.1.dylib`])
            }
//             macDeployQtArgs.push(`-executable=${libDir}/lib${lib}.1.dylib`);
        }
        
//         the-libs.framework/Versions/1/the-libs
        
        //Embed frameworks
        let embedFrameworks = core.getInput("embed-frameworks-mac").split(" ");
        embedFrameworks.push("the-libs");
        
        let fwDir = `${bundlePath}/Contents/Frameworks`;
        await io.rmRF(libDir);
        await io.mkdirP(libDir);
        
        let qtFwPath = exec
        
        for (let framework of embedFrameworks) {
            if (framework == "") continue;
            
            await exec.exec('cp', [`${installDir}/${framework}.framework`, `${fwDir}/${framework}.framework`]);
            await exec.exec("install_name_tool", ["-change", `${framework}.framework/Versions/1/${framework}`, `@executable_path/../Frameworks/${lib}.framework/Versions/Current/${framework}`, `${bundlePath}/Contents/MacOS/${executableName}`])
            if (lib != "the-libs") {
                await exec.exec("install_name_tool", ["-change", `the-libs.framework/Versions/1/the-libs`, `@executable_path/../Frameworks/the-libs.framework/Versions/Current/the-libs`, `${fwDir}/${framework}.framework/Versions/Current/${framework}`])
            }
//             macDeployQtArgs.push(`-executable=${libDir}/lib${lib}.1.dylib`);
        }
        
        await exec.exec("macdeployqt", macDeployQtArgs);
        
        if (fs.existsSync(`${bundlePath}/Contents/PlugIns/styles/libContemporary.dylib`)) {
                    await exec.exec("install_name_tool", ["-change", `the-libs.framework/Versions/1/the-libs`, `@executable_path/../Frameworks/the-libs.framework/Versions/Current/the-libs`, `${bundlePath}/Contents/PlugIns/styles/libContemporary.dylib`]);
        }
        
        await new Promise(function(res, rej) {
            let dmg = appdmg({
                target: `${process.env["HOME"]}/${executableName}.dmg`,
                basepath: `${process.cwd()}/build`,
                specification: {
                    title: executableName,
                    icon: "dmgicon.icns",
                    background: "app-dmg-background.png",
                    "icon-size": 48,
                    window: {
                        size: {
                            width: 600,
                            height: 420
                        }
                    },
                    contents: [
                        {
                            x: 125,
                            y: 225,
                            type: "file",
                            path: bundlePath
                        },
                        {
                            x: 470,
                            y: 225,
                            type: "link",
                            path: "/Applications"
                        }
                    ]
                }
            });
            
            dmg.on('finish', res);
            dmg.on('error', rej);
        });
        
        core.setOutput("image-path", `${process.env["HOME"]}/${executableName}.dmg`);
        core.setOutput("asset-name", `${process.env["GITHUB_REPOSITORY"].substr(process.env["GITHUB_REPOSITORY"].lastIndexOf("/") + 1)}-macOS.dmg`);
        core.setOutput("asset-content-type", "application/x-apple-diskimage");
    } else if (process.platform === 'win32') {
        //TODO
        core.setFailed("Not running on a supported platform.");
        return;
    } else {
        //Fail
        core.setFailed("Not running on a supported platform.");
        return;
    }
})().catch(function(error) {
    console.log(error);
    core.setFailed("Catastrophic Failure");
});
