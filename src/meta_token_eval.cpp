#include "meta_token_eval.hpp"

#include "common.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void fail_eval(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

std::string token_return_forms(const std::string& input_name) {
    return "token_stream meta function bodies currently allow only an empty body, `return " +
           input_name +
           ";` identity body, tokens!(...) token output, or expression-only `if` token branching with supported token_stream conditions";
}

bool is_input_name(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Name && expr.name == input_name;
}

bool is_token_constructor(const Expr& expr) {
    return expr.kind == ExprKind::MacroCall && expr.name == "tokens";
}

bool supported_token_constructor(const Expr& expr, std::string& reason) {
    if (!is_token_constructor(expr)) return false;
    if (!expr.macro_tokens || expr.macro_tokens->empty()) {
        reason = "tokens! token_stream constructor requires one or more output tokens";
        return false;
    }
    return true;
}

bool supported_tokens_empty_call(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Call &&
           expr.name == "tokens_empty" &&
           !expr_operand(expr) &&
           expr.args.size() == 1 &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           is_input_name(*expr.args[0], input_name);
}

bool supported_tokens_empty_method(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == "is_empty" &&
           expr.args.empty() &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_input_name(*expr_operand(expr), input_name);
}

bool supported_tokens_count_call(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Call &&
           expr.name == "tokens_count" &&
           !expr_operand(expr) &&
           expr.args.size() == 1 &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           is_input_name(*expr.args[0], input_name);
}

bool supported_tokens_len_method(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == "len" &&
           expr.args.empty() &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_input_name(*expr_operand(expr), input_name);
}

bool is_string_literal(const Expr& expr) {
    return expr.kind == ExprKind::String;
}

bool all_string_literals(const std::vector<ExprPtr>& args, std::size_t first) {
    if (first >= args.size()) return false;
    for (std::size_t i = first; i < args.size(); ++i) {
        if (!is_string_literal(*args[i])) return false;
    }
    return true;
}

bool supported_tokens_starts_with_call(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Call &&
           expr.name == "tokens_starts_with" &&
           !expr_operand(expr) &&
           expr.args.size() >= 2 &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           is_input_name(*expr.args[0], input_name) &&
           all_string_literals(expr.args, 1);
}

bool supported_tokens_starts_with_method(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == "starts_with" &&
           !expr.args.empty() &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_input_name(*expr_operand(expr), input_name) &&
           all_string_literals(expr.args, 0);
}

bool token_starts_with(const std::vector<Token>& input_tokens,
                       const std::vector<ExprPtr>& parts,
                       std::size_t first) {
    if (input_tokens.size() < parts.size() - first) return false;
    for (std::size_t i = first; i < parts.size(); ++i) {
        if (input_tokens[i - first].text != parts[i]->string_value) return false;
    }
    return true;
}

bool supported_token_integer_expr(const Expr& expr,
                                  const std::string& input_name,
                                  std::string& reason) {
    switch (expr.kind) {
        case ExprKind::Integer:
            return !expr.int_negative;
        case ExprKind::Call:
            if (supported_tokens_count_call(expr, input_name)) return true;
            break;
        case ExprKind::MethodCall:
            if (supported_tokens_len_method(expr, input_name)) return true;
            break;
        default:
            break;
    }
    reason = "token_stream meta integer conditions currently support only non-negative integer literals, tokens_count(" +
             input_name + "), or " + input_name + ".len()";
    return false;
}

std::uint64_t eval_token_integer_expr(const Expr& expr,
                                      const std::string& input_name,
                                      const std::vector<Token>& input_tokens) {
    switch (expr.kind) {
        case ExprKind::Integer:
            if (!expr.int_negative) return expr.int_value;
            break;
        case ExprKind::Call:
            if (supported_tokens_count_call(expr, input_name)) return input_tokens.size();
            break;
        case ExprKind::MethodCall:
            if (supported_tokens_len_method(expr, input_name)) return input_tokens.size();
            break;
        default:
            break;
    }
    fail_eval(expr.loc, "internal error: unsupported token_stream meta integer expression");
}

bool is_token_integer_comparison(TokenKind op) {
    return op == TokenKind::EqEq ||
           op == TokenKind::BangEq ||
           op == TokenKind::Less ||
           op == TokenKind::LessEq ||
           op == TokenKind::Greater ||
           op == TokenKind::GreaterEq;
}

bool supported_token_condition_expr(const Expr& expr,
                                    const std::string& input_name,
                                    std::string& reason) {
    switch (expr.kind) {
        case ExprKind::Bool:
            return true;
        case ExprKind::Unary:
            if (expr.op == TokenKind::Bang && expr_operand(expr)) {
                return supported_token_condition_expr(*expr_operand(expr), input_name, reason);
            }
            break;
        case ExprKind::Binary:
            if ((expr.op == TokenKind::AmpAmp || expr.op == TokenKind::PipePipe) &&
                expr_left(expr) &&
                expr_right(expr)) {
                return supported_token_condition_expr(*expr_left(expr), input_name, reason) &&
                       supported_token_condition_expr(*expr_right(expr), input_name, reason);
            }
            if (is_token_integer_comparison(expr.op) &&
                expr_left(expr) &&
                expr_right(expr)) {
                return supported_token_integer_expr(*expr_left(expr), input_name, reason) &&
                       supported_token_integer_expr(*expr_right(expr), input_name, reason);
            }
            break;
        case ExprKind::Call:
            if (supported_tokens_empty_call(expr, input_name)) return true;
            if (supported_tokens_starts_with_call(expr, input_name)) return true;
            break;
        case ExprKind::MethodCall:
            if (supported_tokens_empty_method(expr, input_name)) return true;
            if (supported_tokens_starts_with_method(expr, input_name)) return true;
            break;
        default:
            break;
    }

    reason = "token_stream meta branch conditions currently support bool literals, !, &&, ||, tokens_empty(" +
             input_name + "), " + input_name + ".is_empty(), and integer comparisons over tokens_count(" +
             input_name + ") or " + input_name + ".len(), plus token-prefix text matching with tokens_starts_with(" +
             input_name + ", \"...\", ...) or " + input_name + ".starts_with(\"...\", ...)";
    return false;
}

bool eval_token_condition_expr(const Expr& expr,
                               const std::string& input_name,
                               const std::vector<Token>& input_tokens) {
    switch (expr.kind) {
        case ExprKind::Bool:
            return expr.bool_value;
        case ExprKind::Unary:
            if (expr.op == TokenKind::Bang && expr_operand(expr)) {
                return !eval_token_condition_expr(*expr_operand(expr), input_name, input_tokens);
            }
            break;
        case ExprKind::Binary:
            if (expr.op == TokenKind::AmpAmp && expr_left(expr) && expr_right(expr)) {
                return eval_token_condition_expr(*expr_left(expr), input_name, input_tokens) &&
                       eval_token_condition_expr(*expr_right(expr), input_name, input_tokens);
            }
            if (expr.op == TokenKind::PipePipe && expr_left(expr) && expr_right(expr)) {
                return eval_token_condition_expr(*expr_left(expr), input_name, input_tokens) ||
                       eval_token_condition_expr(*expr_right(expr), input_name, input_tokens);
            }
            if (is_token_integer_comparison(expr.op) && expr_left(expr) && expr_right(expr)) {
                std::uint64_t left = eval_token_integer_expr(*expr_left(expr), input_name, input_tokens);
                std::uint64_t right = eval_token_integer_expr(*expr_right(expr), input_name, input_tokens);
                switch (expr.op) {
                    case TokenKind::EqEq:
                        return left == right;
                    case TokenKind::BangEq:
                        return left != right;
                    case TokenKind::Less:
                        return left < right;
                    case TokenKind::LessEq:
                        return left <= right;
                    case TokenKind::Greater:
                        return left > right;
                    case TokenKind::GreaterEq:
                        return left >= right;
                    default:
                        break;
                }
            }
            break;
        case ExprKind::Call:
            if (supported_tokens_empty_call(expr, input_name)) return input_tokens.empty();
            if (supported_tokens_starts_with_call(expr, input_name)) {
                return token_starts_with(input_tokens, expr.args, 1);
            }
            break;
        case ExprKind::MethodCall:
            if (supported_tokens_empty_method(expr, input_name)) return input_tokens.empty();
            if (supported_tokens_starts_with_method(expr, input_name)) {
                return token_starts_with(input_tokens, expr.args, 0);
            }
            break;
        default:
            break;
    }
    fail_eval(expr.loc, "internal error: unsupported token_stream meta condition");
}

bool expression_only_branch(const std::vector<StmtPtr>& body) {
    return body.empty();
}

} // namespace

std::vector<Token> substitute_meta_input_tokens(const std::vector<Token>& constructor_tokens,
                                                const std::string& input_name,
                                                const std::vector<Token>& input_tokens) {
    std::vector<Token> expanded;
    for (const Token& token : constructor_tokens) {
        if (token.kind == TokenKind::Identifier && token.text == input_name) {
            expanded.insert(expanded.end(), input_tokens.begin(), input_tokens.end());
            continue;
        }
        expanded.push_back(token);
    }
    return expanded;
}

bool is_supported_meta_token_return_expr(const Expr& expr,
                                         const std::string& input_name,
                                         std::string& reason) {
    if (supported_token_constructor(expr, reason)) return true;

    if (expr.kind == ExprKind::If) {
        if (!expr_if_condition(expr) || !expr_if_then_value(expr) || !expr_if_else_value(expr)) {
            reason = "malformed token_stream meta branch";
            return false;
        }
        if (expr_if_condition_pattern(expr)) {
            reason = "token_stream meta branch conditions cannot use if-let patterns";
            return false;
        }
        if (!expression_only_branch(expr_if_then_body(expr)) ||
            !expression_only_branch(expr_if_else_body(expr))) {
            reason = "token_stream meta branches currently require expression-only arms with no statement bodies";
            return false;
        }
        if (!supported_token_condition_expr(*expr_if_condition(expr), input_name, reason)) return false;
        if (!is_supported_meta_token_return_expr(*expr_if_then_value(expr), input_name, reason)) return false;
        if (!is_supported_meta_token_return_expr(*expr_if_else_value(expr), input_name, reason)) return false;
        return true;
    }

    if (reason.empty()) reason = token_return_forms(input_name);
    return false;
}

std::vector<Token> evaluate_meta_token_return_expr(const Expr& expr,
                                                   const std::string& input_name,
                                                   const std::vector<Token>& input_tokens) {
    if (is_token_constructor(expr)) {
        if (!expr.macro_tokens || expr.macro_tokens->empty()) {
            fail_eval(expr.loc, "tokens! token_stream constructor requires one or more output tokens");
        }
        return substitute_meta_input_tokens(*expr.macro_tokens, input_name, input_tokens);
    }

    if (expr.kind == ExprKind::If) {
        if (!expr_if_condition(expr) || !expr_if_then_value(expr) || !expr_if_else_value(expr)) {
            fail_eval(expr.loc, "malformed token_stream meta branch");
        }
        bool take_then = eval_token_condition_expr(*expr_if_condition(expr), input_name, input_tokens);
        return evaluate_meta_token_return_expr(
            take_then ? *expr_if_then_value(expr) : *expr_if_else_value(expr),
            input_name,
            input_tokens);
    }

    fail_eval(expr.loc, "internal error: unsupported token_stream meta return expression");
}

} // namespace ari
