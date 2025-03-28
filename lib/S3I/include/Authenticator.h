#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <CustomLogging.h>
#include <WiFiClientSecure.h>

#include <vector>

#include "DefaultCertificate.h"

namespace S3I {
struct Credentials {
    const char* Id;
    const char* Secret;

    Credentials(const char* Id, const char* Secret) : Id(Id), Secret(Secret) {}
    Credentials() = default;
    Credentials(Credentials& other) : Id(other.Id), Secret(other.Secret) {}
};

struct IDPServer {
    const char* Server;
    const char* Path;
    const char* Certificate;

    IDPServer(const char* Server, const char* Path,
              const char* Certificate = nullptr)
        : Server(Server), Path(Path) {
        if (Certificate == nullptr) {
            this->Certificate = GetDefaultCertificate();
        } else {
            this->Certificate = Certificate;
        }
    }
    IDPServer() = default;
    IDPServer(IDPServer& other)
        : Server(other.Server),
          Path(other.Path),
          Certificate(other.Certificate) {}
};

struct Token {
    Token(const char* AccessToken, const char* RefreshToken,
          const char* TokenType, uint32_t ExpiresAt, bool Valid = true)
        : AccessToken(AccessToken),
          RefreshToken(RefreshToken),
          TokenType(TokenType),
          ExpiresAt(ExpiresAt),
          Valid(Valid) {}

    Token(const char* Response);  // Is not const because then ArduinoJson
                                  // does not need to copy the Response

    Token() = default;

    Token(const Token& other)
        : AccessToken(other.AccessToken),
          RefreshToken(other.RefreshToken),
          TokenType(other.TokenType),
          ExpiresAt(other.ExpiresAt),
          Valid(other.Valid) {}

    const char* AccessToken;
    const char* RefreshToken;
    const char* TokenType;
    uint32_t ExpiresAt;

    String GetFullToken() const {
        return String(TokenType) + " " + String(AccessToken);
    }

    // Personally, I think this is a perfect use case for operator
    // overloading. You might disagree, and that's fine. But honestly, just
    // look at it! Isn't it elegant? Beautiful, even? Maybe you don’t see it
    // yet, but trust me—it’s pretty, isn't it? ISN'T IT? Why am I rambling
    // on about this? Well, I’m not entirely sure. In fact, the more I think
    // about it, the more I realize I don’t really know. I mean, why did I
    // even write this comment? I don’t know. Should I stop writing it? You
    // guessed it—I still don’t know. It’s a mystery, really. An endless
    // loop of uncertainty. Oh man, I really don’t know where this is going.
    // Should I stop? I mean, maybe, but again, I really don’t know. Someone
    // else might chime in: “Please, for the love of all that is good, just
    // stop.” And I would respond, “I don’t know if I can do that.”
    //
    // "Stop now!" they might insist.
    // "But should I?" I ask, unsure, conflicted... confused.
    // "Yes, STOP!" they yell.
    // "I don’t know…" I reply once again, in my ever-present state of
    // uncertainty. Am I annoying you at this point? Probably. But honestly,
    // I don’t know. And that brings me back to the original question: Why
    // am I writing this? What was the purpose? What was the plan? Once
    // again, I find myself without an answer. I really, truly don’t know.
    //
    // Alright, time to refocus. Back to the code (finally):
    operator bool() const {
        bool TokenNotExpired = ExpiresAt > millis();
        bool TokenNotEmpty = AccessToken != nullptr;
        return Valid && TokenNotExpired && TokenNotEmpty;
    }
    // See? Practical, simple, clean. So, was all that worth it?
    // I don’t know. But what I do know is that this operator overload gets
    // the job done. And in the end, that’s all that matters, right? Right?
    // (Please say yes…) (In parts courtesy of ChatGPT and Copilot)

   private:
    bool Valid = false;
};

class Authenticator {
   public:
    Authenticator(S3I::IDPServer IdpServer, S3I::Credentials Credentials)
        : m_IDPServer(IdpServer), m_Credentials(Credentials) {}
    Authenticator() = default;

    Token Authenticate();
    // void RefreshToken();

    // First check if the token is still valid, if not, get a new one
    Token GetToken();

   private:
    Credentials m_Credentials;
    IDPServer m_IDPServer;

    Token m_LatestToken;
};
}  // namespace S3I
