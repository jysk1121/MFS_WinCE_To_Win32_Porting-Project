#!/usr/bin/env python3
import argparse
import os
import re
from threading import Thread

from version_helper import get_winatm_info

from builder import WinCEBuilder
from builder.models import *

from manifest_builder.serializers import XmlSerializer
from manifest_builder.signing import HMACSigner, RSASigner, Pkcs11Signer
from manifest_builder.manifest import Manifest
from manifest_builder.builders import (
    ManifestBuilder, write_manifest_sig, write_manifest_p7
)

from publisher import Publisher, PublishOptions
from publisher.storages import FileStorage, S3Storage

import time


def timeit(method):
    def timed(*args, **kw):
        ts = time.time()
        result = method(*args, **kw)
        te = time.time()
        if 'log_time' in kw:
            name = kw.get('log_name', method.__name__.upper())
            kw['log_time'][name] = int((te - ts) * 1000)
        else:
            print('%r  %2.2f ms' % \
                  (method.__name__, (te - ts) * 1000))
        return result
    return timed


def sanitize_version(version_number):
    version = version_number
    
    # Detect hotfix version and fix it!
    p = re.compile(r'[0-9]{1,2}\.[0-9]{1,2}\.[0-9]{1,2}.*')
    if p.match(version):
        # convert 06.02.01.x -> 06.02.01
        minor_version = re.compile(r'[0-9]{1,2}\.[0-9]{1,2}\.[0-9]{1,2}')
        return minor_version.match(version)[0]

    if version.rfind('/') != -1:
        version = version[version.rfind('/') + 1:]
        version = re.sub(r'[^a-zA-Z0-9_\.\-]', '', version)

    return version


def _get_model_for_name(modelname):
    if modelname == "1500SE": return SE1500()
    elif modelname == "1800SE": return SE1800()
    elif modelname == '2700SE': return SE2700()
    elif modelname == '2700T': return SE2700T()
    elif modelname == '2800SE': return SE2800()
    elif modelname == '2800T': return SE2800T()
    elif modelname == '5200SE': return SE5200()

    raise Exception(f"No model named {modelname}")


def _get_all_models():
    return [SE1500(), SE1800(), SE2700(), SE2700T(), SE2800(), SE2800T(), SE5200()]


@timeit
def build(args):
    """
    Runs the builder with the provided parameters

    # Example: 'build.py build -v 06.01.40 -l V -t MX -V 60 -m 1500SE -m 2800SE -m 2700SE'
    # Example: 'build.py build -v 06.02.00 -l P -t US -V 70 -o ./my_build_output -s ../Projects/MoniPlusCE2

    """

    if not os.path.isabs(args.source):
        args.source = os.path.abspath(args.source)

    if not os.path.isabs(args.output):
        args.output = os.path.abspath(args.output)

    if not os.path.isdir(args.output):
        os.makedirs(args.output)

    winatm_path = os.path.join(args.source + f"\\exerele_60_{args.territory}\\WinAtm.exe")
    if os.path.isfile(winatm_path):
        (customer, _, version, product_version) = get_winatm_info(winatm_path)
        args.lineage = customer
    else:
        raise Exception("WinAtm.exe not found!!!")

    builder = WinCEBuilder(
        args.territory,  # Territory
        sanitize_version(args.version),  # Version
        args.lineage,  # Lineage
        args.output,  # Output directory
        args.source,  # Source directory
        args.wince_ver,  # WinCE version
    )

    if args.models is not None:
        models = []
        threads = []

        for modelname in args.models:
            model = _get_model_for_name(modelname)
            models.append(model)

            # Make Master.zips in parallel using thread
            th = Thread(target=builder.package_zip, args=([model], False))
            threads.append(th)
        
        for th in threads:
            th.start()
        for th in threads:
            th.join()

    else:
        models = _get_all_models()

    if len(models) > 1:
        # If there was only one mode, then we don't need a combined build
        output = builder.package_zip(models, clean=False)

@timeit
def sign(args):
    """
    Signs the build and creates a manifest file

    """
    create_pkcs7 = args.file_version == '1.1.0'

    default_cert_dir = os.path.join(args.cert_dir, "default")

    extra_signers = []
    if args.sig_type == 'hmac':
        signer = HMACSigner(args.secret)
    elif args.sig_type == 'rsa':
        signer = RSASigner(args.key_file, default_cert_dir)
    elif args.sig_type == 'nha_hsm':
        signer = RSASigner(args.key_file, default_cert_dir)  # Base signer for V1.0.0 compatibility
        for serial in args.cert_serials.split(','):
            serial_cert_dir = os.path.join(args.cert_dir, serial)

            password = os.getenv("HSM_CRYPTO_OP_PIN", None)
            s = Pkcs11Signer(serial, password, serial_cert_dir)
            extra_signers.append(s)

        args.sig_type = "rsa"  # This must be changed for compatbility with the 1.0.0

    # HACK: Yeah... This need to be a better solution
    winatm_path = "WinAtm.exe"
    for folder, subfolders, files in os.walk(args.source_dir):
        if winatm_path in files and folder.lower().endswith(f'_{args.territory}'.lower()):    # exerele_60_[TR]\WinAtm.exe
            winatm_path = os.path.join(folder, winatm_path)
            break

    if os.path.isfile(winatm_path):
        (customer, _, version, product_version) = get_winatm_info(winatm_path)
        args.lineage = customer
    else:
        raise Exception("WinAtm.exe not found!!!")

    manifest = Manifest(
        args.lineage, 
        args.territory,
        sanitize_version(args.version),
        args.model,
        args.severity,
        args.type,
        args.sig_type,
        args.file_version,
    )

    builder = ManifestBuilder(
        signer,
        args.input_files,
        manifest,
        extra_signers=extra_signers
    )

    print(f'Signing packages for {args.lineage}{args.territory}{args.version} - {args.model}...')

    builder.build_manifest()

    XmlSerializer().serialize(builder.manifest, args.manifest)

    write_manifest_sig(args.manifest, signer)

    if create_pkcs7:
        write_manifest_p7(args.manifest, extra_signers + [signer])

    print(args.model + ' done')

def publish(args):
    """
    Publishes the package to the remote software repository

    """

    if args.store_type == "file":
        store = FileStorage(args.repo_root)
    elif args.store_type == 's3':
        store = S3Storage(args.bucket_name, args.repo_root)

    options = PublishOptions()
    options.hives = args.hives.split(' ')
    options.local_packge_dir = args.package_dir
    options.populate_from_manifest()
    options.latest = args.latest

    publisher = Publisher(store, options)
    publisher.publish(not args.hide)

if __name__ == '__main__':
    base_dir = os.getcwd()
    base_dir = base_dir.replace("\\DAT\\PostBuild\\APBuilder", "")
    parser = argparse.ArgumentParser(description='Perform various build tasks for the AP software')
    subparsers = parser.add_subparsers(dest='command', required=True)

    #
    # Build parser sub-command
    #

    source_dir = base_dir
    output_dir = os.path.join(base_dir, 'out')

    # Example: 'build.py build -v 06.01.40 -l V -t MX -m 1500SE -m 2800SE -m 2700SE'
    # Example: 'build.py build -v 06.02.00 -l P -t US -o ./my_build_output -s ../Projects/MoniPlusCE2' -O 70
    build_parser = subparsers.add_parser('build', help='Build the ZIP files for deployment')
    build_parser.set_defaults(func=build)
    build_parser.add_argument('-o', '--output', default=output_dir, help='The output directory for the ZIP file')
    build_parser.add_argument('-s', '--source', default=source_dir, help='The source project directory containing the ExeRele_* directory')
    build_parser.add_argument('-t', dest='territory', help='The territory of the build', required=True)
    build_parser.add_argument('-m', dest='models', action='append', help='Model for which to build the ZIP')
    build_parser.add_argument('-O', dest='wince_ver', default='60', help='The WinCE OS version [60, 70]')
    build_parser.add_argument('-v', dest='version', help='Package version number', required=True)

    #
    # Manifest create command
    #
    # Example: 'build.py sign -o ..\..\..\UpdateFiles\1500SE\manifest.xml   -t $territory -m 1500SE -S $severity -T $update_type -d rsa -k mxkey.pem ..\..\..\UpdateFiles\1500SE'
    manifest_parser = subparsers.add_parser('sign', help='Create a manifest with signatures for the provided inputs')
    manifest_parser.set_defaults(func=sign)
    manifest_parser.add_argument('-s', dest='secret', help='The signing secret')
    manifest_parser.add_argument('-o', dest='manifest', help='The output manifest file')
    manifest_parser.add_argument('-t', dest='territory', help='The territory, i.e. US, MX, CA, etc.')
    manifest_parser.add_argument('-m', dest='model', help='The model of the ATM')
    manifest_parser.add_argument('-S', dest='severity', help='The severity of the release [0-9]')
    manifest_parser.add_argument('-T', dest='type', help='The type of release [bugfix, security, feature]')
    manifest_parser.add_argument('-d', dest='sig_type', help='The type of signature [hmac256, rsa256, nha_hsm]')
    manifest_parser.add_argument('-k', dest='key_file', help='The keyfile to use for the RSA signature')
    manifest_parser.add_argument('-c', dest='cert_serials', help='A list certificate serial numbers to sign the manifest (only for nha_hsm). Comma-separated')
    manifest_parser.add_argument('-V', dest='version', help='Package version number', required=True)
    manifest_parser.add_argument('-v', dest='file_version', default='1.0.0', help='The version of the manifest format to use')
    manifest_parser.add_argument('-f', dest='cert_dir', help='The directory in which the HSM signer should search for certificates')
    manifest_parser.add_argument('input_files', help='The update files to add to the manifest')
    manifest_parser.add_argument('-src', '--source', dest='source_dir', default=source_dir, help='The source project directory containing the ExeRele_* directory')

    #
    # Publish command
    #

    publish_parser = subparsers.add_parser('publish', help='Publish the software version to the remote repo')
    publish_parser.set_defaults(func=publish)
    publish_parser.add_argument('-H', dest='hives', help='A space-separated list of hives', required=True)
    publish_parser.add_argument('-p', dest='package_dir', help='The local package directory', required=True)
    publish_parser.add_argument('-r', dest='repo_root', help='The root of the repository', default='')
    publish_parser.add_argument('-l', dest='latest', action='store_true', help='Publish the package under the latest tag in addition to the version tag')
    publish_parser.add_argument('-s', dest='hide', action='store_true', help='Don\'t publish the build into the contents')

    publish_subparsers = publish_parser.add_subparsers(dest="store_type", help='The type of storage backend to use', required=True)

    # File command
    file_parser = publish_subparsers.add_parser('file', help='Invoke a file storage backend for the publish operations')

    # S3 command
    s3_parser = publish_subparsers.add_parser('s3', help="Invoke an AWS S3 storage backend for the publish operations")
    s3_parser.add_argument('-b', dest='bucket_name', help='The name of the S3 bucket', required=True)


    # Process args and run script
    args = parser.parse_args()
    args.func(args)
