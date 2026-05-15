#include "meta_token_eval.hpp"

#include "common.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void fail_eval(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

std::string token_return_forms(const std::string& input_name) {
    return "token_stream meta function bodies currently allow only an empty body, `return " +
           input_name +
           ";` identity body, tokens!(...) token output, token slice/capture extraction, or expression-only `if` token branching with supported token_stream conditions";
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

bool is_non_negative_integer_literal(const Expr& expr) {
    return expr.kind == ExprKind::Integer && !expr.int_negative;
}

bool is_capture_name_start(char ch) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
}

bool is_capture_name_continue(char ch) {
    return is_capture_name_start(ch) || (ch >= '0' && ch <= '9');
}

bool valid_capture_name(const std::string& name) {
    if (name.empty() || !is_capture_name_start(name.front())) return false;
    for (std::size_t i = 1; i < name.size(); ++i) {
        if (!is_capture_name_continue(name[i])) return false;
    }
    return true;
}

bool capture_marker_name(const std::string& part, std::string& name) {
    if (part.size() < 2 || part.front() != '$') return false;
    name = part.substr(1);
    return valid_capture_name(name);
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

bool supported_tokens_ends_with_call(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Call &&
           expr.name == "tokens_ends_with" &&
           !expr_operand(expr) &&
           expr.args.size() >= 2 &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           is_input_name(*expr.args[0], input_name) &&
           all_string_literals(expr.args, 1);
}

bool supported_tokens_ends_with_method(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == "ends_with" &&
           !expr.args.empty() &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_input_name(*expr_operand(expr), input_name) &&
           all_string_literals(expr.args, 0);
}

bool supported_tokens_wrapped_by_call(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Call &&
           expr.name == "tokens_wrapped_by" &&
           !expr_operand(expr) &&
           expr.args.size() == 3 &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           is_input_name(*expr.args[0], input_name) &&
           is_string_literal(*expr.args[1]) &&
           is_string_literal(*expr.args[2]);
}

bool supported_tokens_wrapped_by_method(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == "wrapped_by" &&
           expr.args.size() == 2 &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_input_name(*expr_operand(expr), input_name) &&
           is_string_literal(*expr.args[0]) &&
           is_string_literal(*expr.args[1]);
}

bool supported_tokens_nth_is_call(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Call &&
           expr.name == "tokens_nth_is" &&
           !expr_operand(expr) &&
           expr.args.size() == 3 &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           is_input_name(*expr.args[0], input_name) &&
           is_non_negative_integer_literal(*expr.args[1]) &&
           is_string_literal(*expr.args[2]);
}

bool supported_tokens_nth_is_method(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == "nth_is" &&
           expr.args.size() == 2 &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_input_name(*expr_operand(expr), input_name) &&
           is_non_negative_integer_literal(*expr.args[0]) &&
           is_string_literal(*expr.args[1]);
}

bool supported_tokens_match_call(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Call &&
           expr.name == "tokens_match" &&
           !expr_operand(expr) &&
           expr.args.size() >= 2 &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           is_input_name(*expr.args[0], input_name) &&
           all_string_literals(expr.args, 1);
}

bool supported_tokens_match_method(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == "matches" &&
           !expr.args.empty() &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_input_name(*expr_operand(expr), input_name) &&
           all_string_literals(expr.args, 0);
}

bool supported_tokens_capture_call(const Expr& expr, const std::string& input_name, std::string& reason) {
    if (expr.kind != ExprKind::Call ||
        expr.name != "tokens_capture" ||
        expr_operand(expr) ||
        expr.args.size() < 3 ||
        !expr_receiver_type_args(expr).empty() ||
        !expr_type_args(expr).empty() ||
        !is_input_name(*expr.args[0], input_name) ||
        !is_string_literal(*expr.args[1]) ||
        !all_string_literals(expr.args, 2)) {
        return false;
    }
    if (!valid_capture_name(expr.args[1]->string_value)) {
        reason = "token_stream capture names must be identifier-like strings";
        return false;
    }
    return true;
}

bool supported_tokens_capture_method(const Expr& expr, const std::string& input_name, std::string& reason) {
    if (expr.kind != ExprKind::MethodCall ||
        expr.name != "capture" ||
        expr.args.size() < 2 ||
        !expr_type_args(expr).empty() ||
        !expr_operand(expr) ||
        !is_input_name(*expr_operand(expr), input_name) ||
        !is_string_literal(*expr.args[0]) ||
        !all_string_literals(expr.args, 1)) {
        return false;
    }
    if (!valid_capture_name(expr.args[0]->string_value)) {
        reason = "token_stream capture names must be identifier-like strings";
        return false;
    }
    return true;
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

bool token_ends_with(const std::vector<Token>& input_tokens,
                     const std::vector<ExprPtr>& parts,
                     std::size_t first) {
    std::size_t count = parts.size() - first;
    if (input_tokens.size() < count) return false;
    std::size_t offset = input_tokens.size() - count;
    for (std::size_t i = first; i < parts.size(); ++i) {
        if (input_tokens[offset + i - first].text != parts[i]->string_value) return false;
    }
    return true;
}

std::string matching_close_text(const std::string& open) {
    if (open == "(") return ")";
    if (open == "{") return "}";
    if (open == "[") return "]";
    return "";
}

std::string matching_open_text(const std::string& close) {
    if (close == ")") return "(";
    if (close == "}") return "{";
    if (close == "]") return "[";
    return "";
}

bool is_close_delimiter_text(const std::string& text) {
    return !matching_open_text(text).empty();
}

bool token_wrapped_by(const std::vector<Token>& input_tokens,
                      const Expr& open_expr,
                      const Expr& close_expr) {
    const std::string& open = open_expr.string_value;
    const std::string& close = close_expr.string_value;
    if (matching_close_text(open) != close) return false;
    if (input_tokens.size() < 2) return false;
    if (input_tokens.front().text != open || input_tokens.back().text != close) return false;

    std::vector<std::string> closing_stack;
    for (std::size_t i = 0; i < input_tokens.size(); ++i) {
        const std::string& text = input_tokens[i].text;
        std::string matching = matching_close_text(text);
        if (!matching.empty()) {
            closing_stack.push_back(matching);
            continue;
        }
        if (is_close_delimiter_text(text)) {
            if (closing_stack.empty() || closing_stack.back() != text) return false;
            closing_stack.pop_back();
            if (closing_stack.empty() && i + 1 != input_tokens.size()) return false;
        }
    }
    return closing_stack.empty();
}

bool token_nth_is(const std::vector<Token>& input_tokens,
                  const Expr& index_expr,
                  const Expr& text_expr) {
    std::uint64_t index = index_expr.int_value;
    return index < input_tokens.size() &&
           input_tokens[static_cast<std::size_t>(index)].text == text_expr.string_value;
}

bool token_matches(const std::vector<Token>& input_tokens,
                   const std::vector<ExprPtr>& parts,
                   std::size_t first) {
    std::size_t count = parts.size() - first;
    if (input_tokens.size() != count) return false;
    std::vector<std::pair<std::string, std::string>> captures;
    for (std::size_t i = first; i < parts.size(); ++i) {
        const std::string& expected = parts[i]->string_value;
        if (expected == "_") continue;
        std::string capture_name;
        if (capture_marker_name(expected, capture_name)) {
            const std::string& actual = input_tokens[i - first].text;
            bool seen = false;
            for (const auto& capture : captures) {
                if (capture.first != capture_name) continue;
                seen = true;
                if (capture.second != actual) return false;
                break;
            }
            if (!seen) captures.push_back({capture_name, actual});
            continue;
        }
        if (input_tokens[i - first].text != expected) return false;
    }
    return true;
}

std::vector<Token> token_capture(const std::vector<Token>& input_tokens,
                                 const std::vector<ExprPtr>& parts,
                                 std::size_t first,
                                 const Expr& capture_name_expr) {
    const std::string& requested_name = capture_name_expr.string_value;
    if (!valid_capture_name(requested_name)) {
        fail_eval(capture_name_expr.loc, "token_stream capture names must be identifier-like strings");
    }
    if (!token_matches(input_tokens, parts, first)) {
        fail_eval(capture_name_expr.loc, "token_stream capture pattern did not match input");
    }

    bool found = false;
    Token captured;
    for (std::size_t i = first; i < parts.size(); ++i) {
        std::string capture_name;
        if (!capture_marker_name(parts[i]->string_value, capture_name) || capture_name != requested_name) continue;
        if (!found) {
            captured = input_tokens[i - first];
            found = true;
            continue;
        }
        if (captured.text != input_tokens[i - first].text) {
            fail_eval(parts[i]->loc, "token_stream capture pattern binds $" + requested_name + " to different tokens");
        }
    }
    if (!found) {
        fail_eval(capture_name_expr.loc, "token_stream capture pattern does not bind $" + requested_name);
    }
    return {captured};
}

bool is_token_integer_arithmetic(TokenKind op) {
    return op == TokenKind::Plus || op == TokenKind::Minus;
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
        case ExprKind::Binary:
            if (is_token_integer_arithmetic(expr.op) &&
                expr_left(expr) &&
                expr_right(expr)) {
                return supported_token_integer_expr(*expr_left(expr), input_name, reason) &&
                       supported_token_integer_expr(*expr_right(expr), input_name, reason);
            }
            break;
        default:
            break;
    }
    reason = "token_stream meta integer conditions currently support only non-negative integer literals, tokens_count(" +
             input_name + "), " + input_name + ".len(), or + / - arithmetic over those values";
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
        case ExprKind::Binary:
            if (is_token_integer_arithmetic(expr.op) && expr_left(expr) && expr_right(expr)) {
                std::uint64_t left = eval_token_integer_expr(*expr_left(expr), input_name, input_tokens);
                std::uint64_t right = eval_token_integer_expr(*expr_right(expr), input_name, input_tokens);
                if (expr.op == TokenKind::Plus) {
                    if (left > std::numeric_limits<std::uint64_t>::max() - right) {
                        fail_eval(expr.loc, "token_stream meta integer expression overflowed");
                    }
                    return left + right;
                }
                if (left < right) {
                    fail_eval(expr.loc, "token_stream meta integer expression cannot become negative");
                }
                return left - right;
            }
            break;
        default:
            break;
    }
    fail_eval(expr.loc, "internal error: unsupported token_stream meta integer expression");
}

bool supported_tokens_slice_call(const Expr& expr,
                                 const std::string& input_name,
                                 std::string& reason) {
    return expr.kind == ExprKind::Call &&
           expr.name == "tokens_slice" &&
           !expr_operand(expr) &&
           expr.args.size() == 3 &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           is_input_name(*expr.args[0], input_name) &&
           supported_token_integer_expr(*expr.args[1], input_name, reason) &&
           supported_token_integer_expr(*expr.args[2], input_name, reason);
}

bool supported_tokens_slice_method(const Expr& expr,
                                   const std::string& input_name,
                                   std::string& reason) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == "slice" &&
           expr.args.size() == 2 &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_input_name(*expr_operand(expr), input_name) &&
           supported_token_integer_expr(*expr.args[0], input_name, reason) &&
           supported_token_integer_expr(*expr.args[1], input_name, reason);
}

std::vector<Token> token_slice(const std::vector<Token>& input_tokens,
                               const Expr& start_expr,
                               const Expr& end_expr,
                               const std::string& input_name) {
    std::uint64_t start = eval_token_integer_expr(start_expr, input_name, input_tokens);
    std::uint64_t end = eval_token_integer_expr(end_expr, input_name, input_tokens);
    if (start > end) {
        fail_eval(start_expr.loc, "token_stream slice start cannot be greater than its end");
    }
    if (end > input_tokens.size()) {
        fail_eval(end_expr.loc, "token_stream slice end is past the input token count");
    }
    auto first = input_tokens.begin() + static_cast<std::vector<Token>::difference_type>(start);
    auto last = input_tokens.begin() + static_cast<std::vector<Token>::difference_type>(end);
    return std::vector<Token>(first, last);
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
            if (supported_tokens_ends_with_call(expr, input_name)) return true;
            if (supported_tokens_wrapped_by_call(expr, input_name)) return true;
            if (supported_tokens_nth_is_call(expr, input_name)) return true;
            if (supported_tokens_match_call(expr, input_name)) return true;
            break;
        case ExprKind::MethodCall:
            if (supported_tokens_empty_method(expr, input_name)) return true;
            if (supported_tokens_starts_with_method(expr, input_name)) return true;
            if (supported_tokens_ends_with_method(expr, input_name)) return true;
            if (supported_tokens_wrapped_by_method(expr, input_name)) return true;
            if (supported_tokens_nth_is_method(expr, input_name)) return true;
            if (supported_tokens_match_method(expr, input_name)) return true;
            break;
        default:
            break;
    }

    reason = "token_stream meta branch conditions currently support bool literals, !, &&, ||, tokens_empty(" +
             input_name + "), " + input_name + ".is_empty(), and integer comparisons over tokens_count(" +
             input_name + ") or " + input_name + ".len() with + / - arithmetic, plus token-prefix text matching with tokens_starts_with(" +
             input_name + ", \"...\", ...) or " + input_name +
             ".starts_with(\"...\", ...), token-suffix text matching with tokens_ends_with(" +
             input_name + ", \"...\", ...) or " + input_name +
             ".ends_with(\"...\", ...), delimiter wrapper checks with tokens_wrapped_by(" +
             input_name + ", \"(\", \")\") or " + input_name +
             ".wrapped_by(\"(\", \")\"), and indexed token text matching with tokens_nth_is(" +
             input_name + ", index, \"...\") or " + input_name +
             ".nth_is(index, \"...\"), plus exact token pattern matching with tokens_match(" +
             input_name + ", \"...\", \"_\", ...) or " + input_name + ".matches(\"...\", \"_\", ...)";
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
            if (supported_tokens_ends_with_call(expr, input_name)) {
                return token_ends_with(input_tokens, expr.args, 1);
            }
            if (supported_tokens_wrapped_by_call(expr, input_name)) {
                return token_wrapped_by(input_tokens, *expr.args[1], *expr.args[2]);
            }
            if (supported_tokens_nth_is_call(expr, input_name)) {
                return token_nth_is(input_tokens, *expr.args[1], *expr.args[2]);
            }
            if (supported_tokens_match_call(expr, input_name)) {
                return token_matches(input_tokens, expr.args, 1);
            }
            break;
        case ExprKind::MethodCall:
            if (supported_tokens_empty_method(expr, input_name)) return input_tokens.empty();
            if (supported_tokens_starts_with_method(expr, input_name)) {
                return token_starts_with(input_tokens, expr.args, 0);
            }
            if (supported_tokens_ends_with_method(expr, input_name)) {
                return token_ends_with(input_tokens, expr.args, 0);
            }
            if (supported_tokens_wrapped_by_method(expr, input_name)) {
                return token_wrapped_by(input_tokens, *expr.args[0], *expr.args[1]);
            }
            if (supported_tokens_nth_is_method(expr, input_name)) {
                return token_nth_is(input_tokens, *expr.args[0], *expr.args[1]);
            }
            if (supported_tokens_match_method(expr, input_name)) {
                return token_matches(input_tokens, expr.args, 0);
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
    if (is_input_name(expr, input_name)) return true;
    if (supported_token_constructor(expr, reason)) return true;

    if (expr.kind == ExprKind::Call && supported_tokens_slice_call(expr, input_name, reason)) return true;
    if (expr.kind == ExprKind::MethodCall && supported_tokens_slice_method(expr, input_name, reason)) return true;
    if (expr.kind == ExprKind::Call && supported_tokens_capture_call(expr, input_name, reason)) return true;
    if (expr.kind == ExprKind::MethodCall && supported_tokens_capture_method(expr, input_name, reason)) return true;

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
    if (is_input_name(expr, input_name)) return input_tokens;

    if (is_token_constructor(expr)) {
        if (!expr.macro_tokens || expr.macro_tokens->empty()) {
            fail_eval(expr.loc, "tokens! token_stream constructor requires one or more output tokens");
        }
        return substitute_meta_input_tokens(*expr.macro_tokens, input_name, input_tokens);
    }

    std::string reason;
    if (supported_tokens_slice_call(expr, input_name, reason)) {
        return token_slice(input_tokens, *expr.args[1], *expr.args[2], input_name);
    }
    if (supported_tokens_slice_method(expr, input_name, reason)) {
        return token_slice(input_tokens, *expr.args[0], *expr.args[1], input_name);
    }
    if (supported_tokens_capture_call(expr, input_name, reason)) {
        return token_capture(input_tokens, expr.args, 2, *expr.args[1]);
    }
    if (supported_tokens_capture_method(expr, input_name, reason)) {
        return token_capture(input_tokens, expr.args, 1, *expr.args[0]);
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
