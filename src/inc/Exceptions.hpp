#pragma once

#include <cstdint>
#include <string>
#include <exception>
#include <cassert>
#include <functional>

#include "AppxWindows.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/sax/SAXParseException.hpp"
#include "xercesc/dom/DOM.hpp"

namespace xPlat {

    static const std::uint32_t ERROR_FACILITY = 0x8BAD0000;   // Facility 2989

    // defines error codes
    enum class Error : std::uint32_t
    {
        //
        // Win32 error codes
        //
        OK                          = 0x00000000,
        NotImplemented              = 0x80004001,
        NoInterface                 = 0x80004002,
        Unexpected                  = 0x8000ffff,
        FileNotFound                = 0x80070002,
        OutOfMemory                 = 0x8007000E,
        NotSupported                = 0x80070032,
        InvalidParameter            = 0x80070057,
        Stg_E_Invalidpointer        = 0x80030009,

        //
        // xPlat specific error codes
        //

        // Basic file errors
        FileOpen                    = ERROR_FACILITY + 0x0001,
        FileSeek                    = ERROR_FACILITY + 0x0002,
        FileRead                    = ERROR_FACILITY + 0x0003,
        FileWrite                   = ERROR_FACILITY + 0x0003,
        FileCreateDirectory         = ERROR_FACILITY + 0x0004,
        FileSeekOutOfRange          = ERROR_FACILITY + 0x0005,

        // Zip format errors
        ZipCentralDirectoryHeader   = ERROR_FACILITY + 0x0011,
        ZipLocalFileHeader          = ERROR_FACILITY + 0x0012,
        Zip64EOCDRecord             = ERROR_FACILITY + 0x0013,
        Zip64EOCDLocator            = ERROR_FACILITY + 0x0014,
        ZipEOCDRecord               = ERROR_FACILITY + 0x0015,
        ZipHiddenData               = ERROR_FACILITY + 0x0016,
        ZipBadExtendedData          = ERROR_FACILITY + 0x0017,

        // Inflate errors
        InflateInitialize           = ERROR_FACILITY + 0x0021,
        InflateRead                 = ERROR_FACILITY + 0x0022,
        InflateCorruptData          = ERROR_FACILITY + 0x0023,
      
        // AppxPackage format errors
        AppxMissingSignatureP7X     = ERROR_FACILITY + 0x0031,
        AppxMissingContentTypesXML  = ERROR_FACILITY + 0x0032,
        AppxMissingBlockMapXML      = ERROR_FACILITY + 0x0033,
        AppxMissingAppxManifestXML  = ERROR_FACILITY + 0x0034,
        AppxDuplicateFootprintFile  = ERROR_FACILITY + 0x0035,

        // Signature errors
        AppxSignatureInvalid        = ERROR_FACILITY + 0x0041,
        AppxCertNotTrusted          = ERROR_FACILITY + 0x0042,

        // Blockmap semantic errors
        BlockMapSemanticError       = ERROR_FACILITY + 0x0051,

        // Parsing facilities.
        XMLException                = ERROR_FACILITY + 0x1000, // Xerces XMLException. 0x8BAD1000 + XMLException error code        
        DOMException                = ERROR_FACILITY + 0x2000, // Xerces DOMException. 0x8BAD2000 + Xerces DOMException error code        
        SAXParseException           = ERROR_FACILITY + 0x3000, // Xerces SAXParseException.
    };

    // Defines a common exception type to throw in exceptional cases.  DO NOT USE FOR FLOW CONTROL!
    // Throwing xPlat::Exception will break into the debugger on chk builds to aid debugging
    class Exception : public std::exception
    {
    public:
        Exception(Error error) : m_code(static_cast<std::uint32_t>(error))
        {}

        Exception(std::uint32_t error) : m_code(0x80070000 + error)
        {}

        Exception(Error error, std::string& message) :
            m_code(static_cast<std::uint32_t>(error)),
            m_message(message)
        {}

        Exception(Error error, const char* message) :
            m_code(static_cast<std::uint32_t>(error)),
            m_message(message)
        {}

        Exception(HRESULT error, std::string& message) :
            m_code(error),
            m_message(message)
        {}

        Exception(HRESULT error, const char* message) :
            m_code(error),
            m_message(message)
        {}

        uint32_t        Code() { return m_code; }
        std::string&    Message() { return m_message; }

        //// copy ctor
        //Exception(Exception& right) : m_code(right.m_code), m_message(right.m_message) { }

        //// User by XercesDOMParser
        //void warning(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
        //{
        //    // Todo add e.getMessage(), e.getColumnNumber() and e.getLineNumber()
        //    throw this;
        //}

        //void error(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
        //{
        //    // Todo add e.getMessage(), e.getColumnNumber() and e.getLineNumber()
        //    throw this;
        //}

        //void fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& exc) override
        //{
        //    // Todo add e.getMessage(), e.getColumnNumber() and e.getLineNumber()
        //    throw this;
        //}

        //void resetErrors() override {}

    protected:
        std::uint32_t   m_code;
        std::string     m_message;
    };

    class Win32Exception : public Exception
    {
    public:
        Win32Exception(DWORD error, std::string& message) :
            Exception(0x80070000 + error, message)
        {}

        Win32Exception(DWORD error, const char* message) :
            Exception(0x80070000 + error, message)
        {}
    };

    class ParsingException : public XERCES_CPP_NAMESPACE::ErrorHandler
    {
    public:
        ParsingException() {};
        ~ParsingException() {};

        void warning(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
        {
            // TODO: add message, line number and column
            throw Exception(xPlat::Error::SAXParseException);
        }

        void error(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
        {
            // TODO: add message, line number and column
            throw Exception(xPlat::Error::SAXParseException);
        }

        void fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
        {
            // TODO: add message, line number and column
            throw Exception(xPlat::Error::SAXParseException);
        }

        void resetErrors() override {}
    };

    // Provides an ABI exception boundary with parameter validation
    template <class Lambda>
    inline HRESULT ResultOf(Lambda lambda)
    {
        HRESULT hr = static_cast<HRESULT>(xPlat::Error::OK);
        try
        {
            lambda();
        }
        catch (xPlat::Exception& e)
        {
            hr = static_cast<HRESULT>(e.Code());
        }
        catch (std::bad_alloc&)
        {
            hr = static_cast<HRESULT>(xPlat::Error::OutOfMemory);
        }
        catch (std::exception&)
        {
            hr = static_cast<HRESULT>(xPlat::Error::Unexpected);
        }
        catch (const XERCES_CPP_NAMESPACE::XMLException& e)
        {
            hr = static_cast<HRESULT>(xPlat::Error::XMLException) +
                static_cast<HRESULT>(e.getCode());
        }
        catch (const XERCES_CPP_NAMESPACE::DOMException& e)
        {
            hr = static_cast<HRESULT>(xPlat::Error::DOMException) +
                static_cast<HRESULT>(e.code);
        }

        return hr;
    }
}

// Helper to make code more terse and more readable at the same time.
#define ThrowErrorIfNot(c, a, m)     \
{                                    \
    if (!(a))                        \
    {                                \
        assert(false);               \
        throw xPlat::Exception(c,m); \
    }                                \
}

#define ThrowWin32ErrorIfNot(c, a, m)       \
{                                           \
    if (!(a))                               \
    {                                       \
        assert(false);                      \
        throw xPlat::Win32Exception(c,m);   \
    }                                       \
}

#define ThrowErrorIf(c, a, m) ThrowErrorIfNot(c,!(a), m)

#define ThrowHrIfFailed(a)                              \
{                                                       \
    HRESULT hr = a;                                     \
    if (FAILED(hr))                                     \
    {   assert(false);                                  \
        throw xPlat::Exception(hr, "COM Call failed");  \
    }                                                   \
}
