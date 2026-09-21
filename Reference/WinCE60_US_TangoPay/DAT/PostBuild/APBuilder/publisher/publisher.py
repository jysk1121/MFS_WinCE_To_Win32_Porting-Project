import os
from dataclasses import dataclass
import xml.etree.ElementTree as ET
from .storages import Storage
from manifest_builder.serializers import XmlSerializer

AP_COMPONENT = 'ap'


@dataclass(init=False)
class PublishOptions:
    """
    Model containing the parameters about how to publish a package

    """

    hives: [str]
    local_packge_dir: str

    version: str = ''
    manifest_version: str = ''
    territory: str = ''
    lineage: str = ''
    model: str = ''
    component: str = AP_COMPONENT
    latest: bool = False

    @classmethod
    def _get_os_model(cls, model):
        if model == '1500SE':
            return 'NH1500SE'
        elif model == '1800SE':
            return 'NH1800SE'
        elif model == '2700SE':
            return 'NH2700'
        elif model == '2700T':
            return 'NH2700T'
        elif model == '2800SE':
            return 'MX2800SE'
        elif model == '2800T':
            return 'MX2800T'
        elif model == '5200SE':
            return 'MX5200SE'
        return ''

    def populate_from_manifest(self):
        serializer = XmlSerializer()
        manifest_path = os.path.join(self.local_packge_dir, "manifest.xml")
        manifest = serializer.deserialize(manifest_path)

        self.lineage = manifest.lineage
        self.manifest_version = manifest.version
        self.version = manifest.version.replace(' ', '')
        self.model = PublishOptions._get_os_model(manifest.model)
        self.territory = manifest.territory

    def _get_hives_path(self):
        path = ''
        for hive in self.hives:
            if hive == '$territory':
                hive = self.territory
            elif hive == '$model':
                hive = self.model
            elif hive == '$lineage':
                hive = self.lineage

            path = os.path.join(path, hive)

        return path

    def component_content_path(self):
        return os.path.join(self._get_hives_path(), 'pkg', self.component, 'contents.xml')

    def package_path(self):
        return os.path.join(self._get_hives_path(), 'pkg', self.component, f'{self.lineage}{self.territory}_{self.version}')

    def latest_package_path(self):
        return os.path.join(self._get_hives_path(), 'pkg', self.component, 'latest')


class Publisher:
    """
    A service which publishes Master.zip and manifests to a remote update repository

    """

    def __init__(self, storage: Storage, options: PublishOptions):
        self.storage = storage
        self.options = options

    def publish(self, add_to_contents=True):
        """
        Publishes the package to the repository

        """
        self._upload_packages()

        if add_to_contents:
            contents = self._get_component_contents()
            if contents == False:
                return
            self._add_version_to_contents(contents)
            self._save_contents(contents)

    def _upload_packages(self):
        """
        Uploads the packages to the repository

        """
        print(f'Publishing packages for {self.options.lineage}{self.options.territory}{self.options.version} - {self.options.model}...', end='')
        dest_path = self.options.package_path()
        for item in os.scandir(self.options.local_packge_dir):
            self.storage.upload_file(item.path, dest_path)

        print('done')

        if self.options.latest:
            print(f'  - Tagging package as latest...', end='')
            self.storage.copy_file(dest_path, self.options.latest_package_path())
            print('done')

    def _get_component_contents(self):
        """
        Gets the XML tree for the contents file

        """
        contents_path = self.options.component_content_path()
        try:
            contents_xml = self.storage.get_file_contents(contents_path)
        except:
            contents_xml = '<contents></contents>'

        if len(contents_xml) == 0:
            # Populate with default information
            contents_xml = '<contents></contents>'

        # i.e. <contents>
        root = ET.fromstring(contents_xml)
        return root

    def _add_version_to_contents(self, xml_elem):
        """
        Registers a new version to the contents object

        """

        version = None
        for package in xml_elem.findall('package'):
            # Ignore invalid tags
            if "version" not in package.attrib:
                continue
            # Check for existing
            if f'{self.options.lineage}{self.options.territory}_{self.options.manifest_version}' == package.text:
                version = package
                break

        if version is None:
            version = ET.Element('package')
            xml_elem.insert(0, version)
        
        version.set('version', self.options.manifest_version)
        version.set('path', f'{self.options.lineage}{self.options.territory}_{self.options.version}')
        version.text = f'{self.options.lineage}{self.options.territory}_{self.options.manifest_version}'

    def _save_contents(self, xml_elem):
        """
        Sets the contents of the remote contents.xml

        """
        contents_path = self.options.component_content_path()
        self.storage.set_xml_file_contents(ET.tostring(xml_elem), contents_path)
