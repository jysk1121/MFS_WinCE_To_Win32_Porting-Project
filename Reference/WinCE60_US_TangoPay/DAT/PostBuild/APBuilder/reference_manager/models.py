from dataclasses import dataclass


@dataclass(init=False)
class RemotePackage:
    name: str
    version: str
    file_type: str
    path: str


@dataclass(init=False)
class Reference:
    name: str
    destination: str
    version: str

    territory: str = None
    customer: str = None
