#include "k.h"

template<class T> T atomtoc();
template<class T> T atomtoc(K x) {
    switch (x->t) {
        case -KB: return dynamic_cast<T>(kb(x->g));
        
        default: throw;
    }
    
}

