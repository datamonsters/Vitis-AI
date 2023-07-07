# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: tensor.proto

from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database

# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


DESCRIPTOR = _descriptor.FileDescriptor(
    name="tensor.proto",
    package="openir",
    syntax="proto3",
    serialized_options=None,
    serialized_pb=b'\n\x0ctensor.proto\x12\x06openir"\xd6\x02\n\x0bTensorProto\x12+\n\x05\x64type\x18\x01 \x01(\x0e\x32\x1c.openir.TensorProto.DataType\x12.\n\x05shape\x18\x02 \x01(\x0b\x32\x1f.openir.TensorProto.TensorShape\x12\x0f\n\x07\x63ontent\x18\x03 \x01(\x0c\x1a\x30\n\x0bTensorShape\x12\x0b\n\x03\x64im\x18\x01 \x03(\x05\x12\x14\n\x0cunknown_rank\x18\x02 \x01(\x08"\xa6\x01\n\x08\x44\x61taType\x12\x0b\n\x07UNKNOWN\x10\x00\x12\x08\n\x04INT8\x10\x01\x12\t\n\x05UINT8\x10\x02\x12\t\n\x05INT16\x10\x03\x12\n\n\x06UINT16\x10\x04\x12\t\n\x05INT32\x10\x05\x12\n\n\x06UINT32\x10\x06\x12\t\n\x05INT64\x10\x07\x12\n\n\x06UINT64\x10\x08\x12\x0b\n\x07\x46LOAT32\x10\t\x12\x0b\n\x07\x46LOAT64\x10\n\x12\t\n\x05\x46LOAT\x10\t\x12\n\n\x06\x44OUBLE\x10\n\x1a\x02\x10\x01\x62\x06proto3',
)


_TENSORPROTO_DATATYPE = _descriptor.EnumDescriptor(
    name="DataType",
    full_name="openir.TensorProto.DataType",
    filename=None,
    file=DESCRIPTOR,
    values=[
        _descriptor.EnumValueDescriptor(
            name="UNKNOWN", index=0, number=0, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="INT8", index=1, number=1, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="UINT8", index=2, number=2, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="INT16", index=3, number=3, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="UINT16", index=4, number=4, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="INT32", index=5, number=5, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="UINT32", index=6, number=6, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="INT64", index=7, number=7, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="UINT64", index=8, number=8, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="FLOAT32", index=9, number=9, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="FLOAT64", index=10, number=10, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="FLOAT", index=11, number=9, serialized_options=None, type=None
        ),
        _descriptor.EnumValueDescriptor(
            name="DOUBLE", index=12, number=10, serialized_options=None, type=None
        ),
    ],
    containing_type=None,
    serialized_options=b"\020\001",
    serialized_start=201,
    serialized_end=367,
)
_sym_db.RegisterEnumDescriptor(_TENSORPROTO_DATATYPE)


_TENSORPROTO_TENSORSHAPE = _descriptor.Descriptor(
    name="TensorShape",
    full_name="openir.TensorProto.TensorShape",
    filename=None,
    file=DESCRIPTOR,
    containing_type=None,
    fields=[
        _descriptor.FieldDescriptor(
            name="dim",
            full_name="openir.TensorProto.TensorShape.dim",
            index=0,
            number=1,
            type=5,
            cpp_type=1,
            label=3,
            has_default_value=False,
            default_value=[],
            message_type=None,
            enum_type=None,
            containing_type=None,
            is_extension=False,
            extension_scope=None,
            serialized_options=None,
            file=DESCRIPTOR,
        ),
        _descriptor.FieldDescriptor(
            name="unknown_rank",
            full_name="openir.TensorProto.TensorShape.unknown_rank",
            index=1,
            number=2,
            type=8,
            cpp_type=7,
            label=1,
            has_default_value=False,
            default_value=False,
            message_type=None,
            enum_type=None,
            containing_type=None,
            is_extension=False,
            extension_scope=None,
            serialized_options=None,
            file=DESCRIPTOR,
        ),
    ],
    extensions=[],
    nested_types=[],
    enum_types=[],
    serialized_options=None,
    is_extendable=False,
    syntax="proto3",
    extension_ranges=[],
    oneofs=[],
    serialized_start=150,
    serialized_end=198,
)

_TENSORPROTO = _descriptor.Descriptor(
    name="TensorProto",
    full_name="openir.TensorProto",
    filename=None,
    file=DESCRIPTOR,
    containing_type=None,
    fields=[
        _descriptor.FieldDescriptor(
            name="dtype",
            full_name="openir.TensorProto.dtype",
            index=0,
            number=1,
            type=14,
            cpp_type=8,
            label=1,
            has_default_value=False,
            default_value=0,
            message_type=None,
            enum_type=None,
            containing_type=None,
            is_extension=False,
            extension_scope=None,
            serialized_options=None,
            file=DESCRIPTOR,
        ),
        _descriptor.FieldDescriptor(
            name="shape",
            full_name="openir.TensorProto.shape",
            index=1,
            number=2,
            type=11,
            cpp_type=10,
            label=1,
            has_default_value=False,
            default_value=None,
            message_type=None,
            enum_type=None,
            containing_type=None,
            is_extension=False,
            extension_scope=None,
            serialized_options=None,
            file=DESCRIPTOR,
        ),
        _descriptor.FieldDescriptor(
            name="content",
            full_name="openir.TensorProto.content",
            index=2,
            number=3,
            type=12,
            cpp_type=9,
            label=1,
            has_default_value=False,
            default_value=b"",
            message_type=None,
            enum_type=None,
            containing_type=None,
            is_extension=False,
            extension_scope=None,
            serialized_options=None,
            file=DESCRIPTOR,
        ),
    ],
    extensions=[],
    nested_types=[_TENSORPROTO_TENSORSHAPE,],
    enum_types=[_TENSORPROTO_DATATYPE,],
    serialized_options=None,
    is_extendable=False,
    syntax="proto3",
    extension_ranges=[],
    oneofs=[],
    serialized_start=25,
    serialized_end=367,
)

_TENSORPROTO_TENSORSHAPE.containing_type = _TENSORPROTO
_TENSORPROTO.fields_by_name["dtype"].enum_type = _TENSORPROTO_DATATYPE
_TENSORPROTO.fields_by_name["shape"].message_type = _TENSORPROTO_TENSORSHAPE
_TENSORPROTO_DATATYPE.containing_type = _TENSORPROTO
DESCRIPTOR.message_types_by_name["TensorProto"] = _TENSORPROTO
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

TensorProto = _reflection.GeneratedProtocolMessageType(
    "TensorProto",
    (_message.Message,),
    {
        "TensorShape": _reflection.GeneratedProtocolMessageType(
            "TensorShape",
            (_message.Message,),
            {
                "DESCRIPTOR": _TENSORPROTO_TENSORSHAPE,
                "__module__": "tensor_pb2"
                # @@protoc_insertion_point(class_scope:openir.TensorProto.TensorShape)
            },
        ),
        "DESCRIPTOR": _TENSORPROTO,
        "__module__": "tensor_pb2"
        # @@protoc_insertion_point(class_scope:openir.TensorProto)
    },
)
_sym_db.RegisterMessage(TensorProto)
_sym_db.RegisterMessage(TensorProto.TensorShape)


_TENSORPROTO_DATATYPE._options = None
# @@protoc_insertion_point(module_scope)
