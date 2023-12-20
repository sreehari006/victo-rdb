add_library(
    engine
    db/db.c
    db/interface/db.h
    collection/collection.c
    collection/interface/collection.h
    file/file_io.c
    file/interface/file_io.h
    vector/vector_io.c
    vector/vector_math.c
    vector/interface/vector_io.h
    vector/interface/vector_io_private.h
    vector/interface/vector_math_private.h
)

