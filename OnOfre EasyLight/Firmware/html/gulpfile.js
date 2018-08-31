/**
 * @file gulpfile.js
 * @description Gulp task automator javascript file.
 * @author Bruno Horta
 * @email brunohorta82@gmail.com
 */
const gulp = require('gulp'),
    fs = require('fs'),
    exec = require('child_process').exec,

    // Browser Sync
    browserSync = require('browser-sync').create()

//gulp sequence
gulpSequence = require('gulp-sequence'),

    //rename files
    rename = require('gulp-rename'),

    //rm files
    rimraf = require('rimraf'),

    //concat files
    concat = require('gulp-concat'),

    //gzip compressor
    gzip = require('gulp-gzip'),

    // intercept
    intercept = require('gulp-intercept');

const SRC_FOLDER = './';
const SRC_ASSETS_FOLDER = SRC_FOLDER + '/assets';
const DEST_HEX_FILES = SRC_FOLDER + '/hex';
const DEST_STATIC_SITE_H = '../code/BH_Pzem_WI_FI';
const STATIC_SITE_FILENAME = 'static_site.h';

/**
 * Serve task.
 * This task is responsible for launching Browser Sync and setting up watchers over the file types involved in the
 * development process. If any changes are detected in one of those files, the build process is triggered and subsequently
 * Browser Sync reloads the application in all connected browsers.
 */
gulp.task('serve', function () {
    // make sure the application is built before launching
    fs.stat(SRC_FOLDER + '/index.html', function (err) {
        if (!err) {
            browserSync.init({
                server: {
                    baseDir: SRC_FOLDER,
                    index: 'index.html'
                }
            });
            // listen for changes in the following file types
            gulp.watch([SRC_FOLDER + '/**/*.js', SRC_FOLDER + '/**/*.html', SRC_FOLDER + '/**/*.css']).on('change', browserSync.reload);
        } else {
            // detect specific errors
            switch (err.code) {
                case 'ENOENT':
                    console.log('\x1b[31mGulp "serve" task error\x1b[0m: There is no build available. ' +
                        'Please, run the command \x1b[32mgulp build\x1b[0m before starting the server ' +
                        'or simply \x1b[32mgulp\x1b[0m.\n');
                    break;
                default:
                    console.log('\x1b[31mGulp "serve" task error\x1b[0m: Unknown error. Details: ', err);
                    break;
            }
        }

    });
});

/**
 * Convert file content to hex in c like
 * 0x12, 0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
 * 0x12, 0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
 * @param file
 * @return file in hex format
 */
function convertFileToHexCLike(file, varName) {
    const hexData = new Buffer(file.contents, 'ascii').toString('hex');
    let hexString = '';

    let breakLine = 0; // Max line Length 16 0x00 chars
    let charsNumber = 0;
    for (let i = 0; i < hexData.length; i++) {
        breakLine++;
        charsNumber++;

        var charHex = hexData.charAt(i).toUpperCase();
        switch (charsNumber) {
            case 1:
                //add 0x in first char
                hexString += '0x' + charHex;
                break;
            case 2:
                //add , to last char
                charsNumber = 0;
                hexString += charHex + ', ';
                break;
        }
        //check if line has 16 chars 0x00
        if (breakLine >= 32) {
            hexString += '\n';
            breakLine = 0;
        }
    }
    const fileContent = 'const uint8_t ' + varName + '[] PROGMEM={' + hexString + '};';

    file.contents = new Buffer(fileContent);
    return file;
}

/**
 * create Hex file ready for concat in final static_site.h file
 * @param file
 * @param varName
 */
function createHexFile(file, varName) {
    return gulp.src(file)
        .pipe(gzip())
        .pipe(intercept(function (file) {
            return convertFileToHexCLike(file, varName);
        }))
        .pipe(rename(function (path) {
            //rename file to .html
            path.basename = varName;
            path.extname = '.hex';
        }))
        .pipe(gulp.dest(DEST_HEX_FILES));
}

/**
 * Clean task.
 * This task is responsible for removing the whole 'hex folder html'
 */
gulp.task('cleanHexFolder', function (callback) {
    rimraf(DEST_HEX_FILES, {}, callback);
});


gulp.task('build_hex_files', function () {
    createHexFile('./jquery.min.js', 'jQuery');
    return createHexFile('./GaugeMeter.js', 'gauge');
});

/**
 *  concat all .hex files and create new static_site.h file in BH_Pzem_WI_FI directory
 */
gulp.task('concat_file_static', function () {
    return gulp.src(DEST_HEX_FILES + '/*.hex')
        .pipe(rename(function (path) {
            console.log(path);
        }))
        .pipe(concat(STATIC_SITE_FILENAME))
        .pipe(gulp.dest(DEST_STATIC_SITE_H));
});

/**
 * build static_site.h file from another files
 */
gulp.task('build_static_site', gulpSequence('build_hex_files', 'concat_file_static', 'cleanHexFolder'));

/**
 * Default task.
 * This task is responsible for bundling and running all tasks associated with the production of the application
 * in a distributable format. This task also starts the application server in development mode.
 */
gulp.task('default', function () {
    gulp.start('serve');
});
